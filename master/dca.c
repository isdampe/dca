#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "scheduler.h"
#include "i2c.h"
#include "tui.h"
#include "efp.h"
#include "dca.h"
#include "log.h"

/**
 * Renders all logs into their appropriate columns.
 */
static void log_render()
{
	char str_buffer[100];

	tui_print_col(&mngr, 1, 0, "System");
	for (int i=0; i<DCA_LOG_MAX_LINES; ++i)
		tui_print_col(&mngr, 1, i + 2, system_log[i]);


	tui_print_col(&mngr, 1, DCA_LOG_MAX_LINES + 3, "--------------------------");
	sprintf(str_buffer, "Progress: %.02f percent", ((float)s.current_schedule / (float)WORK_MAX_REQUESTS) * 100);
	tui_print_col(&mngr, 1, DCA_LOG_MAX_LINES + 5, str_buffer);

	for (int i=0; i<2; ++i)
	{
		sprintf(str_buffer, "Solved (%s): %i", i == 0 ? "photon" : "mbed", solved_by[i]);
		tui_print_col(&mngr, 1, DCA_LOG_MAX_LINES + 6 + 2 * i, str_buffer);
		sprintf(str_buffer, "Errors (%s): %i", i == 0 ? "photon" : "mbed", error_by[i]);
		tui_print_col(&mngr, 1, DCA_LOG_MAX_LINES + 7 + 2 * i, str_buffer);
	}

	tui_print_col(&mngr, 2, 0, "Result (digits of Pi)");
	for (int i=0; i<DCA_LOG_MAX_LINES; ++i)
		tui_print_col(&mngr, 2, i + 2, results_log[i]);

	tui_print_col(&mngr, 3, 0, "I2C regis.  A  B  C  D  E");
	tui_print_col(&mngr, 3, 1, "-------------------------");
	for (int i=0; i<DCA_LOG_MAX_LINES; ++i)
		tui_print_col(&mngr, 3, i + 2, i2c_log[i]);

	tui_print_borders(&mngr);
	refresh();
}

/**
 * Reset the DCA static values for recomputation.
 */
void dca_reset()
{
	for (int i=0; i<2; ++i)
	{
		error_by[i] = 0;
		solved_by[i] = 0;
		checksum_counter[i] = 0;
	}
	for (int i=0; i<WORK_STEP_SIZE * WORK_MAX_REQUESTS; ++i)
		results[i] = 0;
	for (int i=0; i<WORK_MAX_REQUESTS; ++i)
		jobs[i] = 0;
}

/**
 * Ensure that the job store memory is initialised to 0x0.
 */
void setup_jobs()
{
	for (int i=0; i<WORK_MAX_REQUESTS; ++i)
		jobs[i] = 0x0;
}

/**
 * Create both the i2c_obj instances for Photon and Mbed.
 * @return True if the operation succeeded, false if errors occured.
 */
bool setup_i2c_slaves()
{
	status = i2c_init(&slave_photon, "/dev/i2c-1", DCA_HW_ADDR_PHOTON, I2C_HW_PHOTON);
	if (status != I2C_STATUS_OK)
	{
		log_append(system_log, "Fatal I2C error on photon:");
		log_append(system_log, i2c_get_status_str(status));
		return false;
	}

	status = i2c_init(&slave_mbed, "/dev/i2c-1", DCA_HW_ADDR_MBED, I2C_HW_MBED);
	if (status != I2C_STATUS_OK)
	{
		log_append(system_log, "Fatal I2C error on mbed:");
		log_append(system_log, i2c_get_status_str(status));
		return false;
	}

	return true;
}

/**
 * Setup and intialise the scheduler instance.
 * @return True if success.
 */
bool setup_scheduler()
{
	s = scheduler_create(2, 25);
	scheduler_set_slave_i2c(&s, 0, &slave_photon, "photon");
	scheduler_set_slave_i2c(&s, 1, &slave_mbed, "mbed");

	return true;
}

/**
 * Gets the next job in the computation session.
 * @return The integer value of the next job index.
 */
int job_get_next()
{
	for (int i=0; i<WORK_MAX_REQUESTS; ++i)
		if (jobs[i] == 0x0)
			return i;

	return -1;
}

/**
 * Automatically dispatches jobs to I2C slaves that are currently not busy.
 */
void auto_dispatch_work()
{
	char str_buffer[100];
	int8_t current_slave = scheduler_get_free_slave_idx(&s, 500);
	slave *sl;

	int current_job = job_get_next();

	if (current_slave >= 0)
	{
		sl = scheduler_get_slave_by_idx(&s, current_slave);
		scheduler_claim_slave(sl);

		//Reset first.
		efp_reset(sl->obj, 100);

		str_buffer[0] = '\0';
		sprintf(str_buffer, "Reset 0x%02x: ", sl->obj->addr);
		i2c_reg_to_string(sl->obj, str_buffer);
		log_append(i2c_log, str_buffer);

		//Create work order.
		if (! efp_order(sl->obj, current_job, EFP_ORDER_TIMEOUT))
		{
			sprintf(str_buffer, "Timeout ordering %s to compute from %i", sl->name, current_job);
			log_append(system_log, str_buffer);
			scheduler_free_slave(sl);

			error_by[(sl->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1]++;
			return;
		}

		str_buffer[0] = '\0';
		sprintf(str_buffer, "Order 0x%02x: ", sl->obj->addr);
		i2c_reg_to_string(sl->obj, str_buffer);
		log_append(i2c_log, str_buffer);


		sl->current_idx = current_job;

		sprintf(str_buffer, "Ordered %s to compute from start index %i\n", sl->name, current_job);
		log_append(system_log, str_buffer);
		s.current_schedule++;
		jobs[current_job] = 0x1;
		checksum_counter[(sl->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1] = 0;
	}
}

/**
 * Checks all busy I2C slaves for results.
 * If they are completed, fetches and stores the results.
 */
void check_results()
{
	char str_buffer[100]; char str_concat_buffer[2];
	for (int8_t i=0; i<s.num_workers; ++i)
	{
		if (! s.slaves[i]->busy)
			continue;

		uint8_t result;

		if (efp_status(s.slaves[i]->obj, &result, 5000) && result == WORK_STEP_SIZE)
		{
			sprintf(str_buffer, "The %s has finished\n", s.slaves[i]->name);
			log_append(system_log, str_buffer);

			uint8_t step_results[WORK_STEP_SIZE];

			if (efp_result_range(s.slaves[i]->obj, step_results, 1, WORK_STEP_SIZE, 100))
			{
				uint32_t idx = (s.slaves[i]->current_idx * WORK_STEP_SIZE);
				sprintf(str_buffer, "Job 0x%02x: ", s.slaves[i]->current_idx);
				for (uint8_t x=0; x<WORK_STEP_SIZE; ++x)
				{
					results[idx + x] = step_results[x];
					sprintf(str_concat_buffer, "%i", step_results[x]);
					strcat(str_buffer, str_concat_buffer);
				}
				log_append(results_log, str_buffer);

				//Free up the slave.
				efp_reset(s.slaves[i]->obj, 100);
				scheduler_free_slave(s.slaves[i]);

				//Solve stats.
				solved_by[(s.slaves[i]->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1]++;

				str_buffer[0] = '\0';
				sprintf(str_buffer, "Reset 0x%02x: ", s.slaves[i]->obj->addr);
				i2c_reg_to_string(s.slaves[i]->obj, str_buffer);
				log_append(i2c_log, str_buffer);
			}
			else
			{
				sprintf(str_buffer, "An error occured fetching results from %s. Releasing to queue\n", s.slaves[i]->name);
				log_append(system_log, str_buffer);
				dca_cancel_job(s.slaves[i]);
			}
			str_buffer[0] = '\0';
			sprintf(str_buffer, "Resu. 0x%02x: ", s.slaves[i]->obj->addr);
			i2c_reg_to_string(s.slaves[i]->obj, str_buffer);
			log_append(i2c_log, str_buffer);

			sprintf(str_buffer, "Status: %i / %i\n", s.current_schedule, WORK_MAX_REQUESTS);
			log_append(system_log, str_buffer);
		}
		else
		{
			checksum_counter[(s.slaves[i]->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1]++;
			if (checksum_counter[(s.slaves[i]->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1] > DCA_CHECKSUM_OVERCOUNT)
			{
				sprintf(str_buffer, "Timed out waiting for result with slave %s. Releasing job to queue.", s.slaves[i]->name);
				log_append(system_log, str_buffer);
				dca_cancel_job(s.slaves[i]);
			}
		}

		str_buffer[0] = '\0';
		sprintf(str_buffer, "Stat. 0x%02x: ", s.slaves[i]->obj->addr);
		i2c_reg_to_string(s.slaves[i]->obj, str_buffer);
		log_append(i2c_log, str_buffer);

		usleep(10000);

	}
}

/**
 * Cancels a job on a given I2C slave.
 * @param sl A pointer to the slave.
 */
void dca_cancel_job(slave *sl)
{
	char str_buffer[100];

	jobs[sl->current_idx] = 0x0;

	efp_reset(sl->obj, 100);
	scheduler_free_slave(sl);

	error_by[(sl->obj->addr == DCA_HW_ADDR_PHOTON) ? 0 : 1]++;

	str_buffer[0] = '\0';
	sprintf(str_buffer, "Reset 0x%02x: ", sl->obj->addr);
	i2c_reg_to_string(sl->obj, str_buffer);
	log_append(i2c_log, str_buffer);
}

/**
 * The main entry-point for a DCA session.
 * @return 0 on success, else 1.
 */
int dca_main()
{
	mngr = tui_create_mgr(3);
	tui_print_borders(&mngr);

	dca_reset();

	//log_append(system_log, "hello world");
	log_append(system_log, "Setting up jobs");

	setup_jobs();

	log_append(system_log, "Setting up I2C devices");
	if (! setup_i2c_slaves())
		return 1;

	log_append(system_log, "Creating scheduler");
	if (! setup_scheduler())
		return 1;

	while (job_get_next() > -1)
	{
		log_render();
		auto_dispatch_work();
		check_results();
	}

	log_append(system_log, "All jobs have been scheduled. Waiting for remaining computations");
	while (scheduler_get_free_slave_idx(&s, 500) == -1)
	{
		log_render();
		check_results();
		usleep(5000);
	}

	tui_end();

	printf("Computation complete\n");
	printf("Pi = 3.");
	for (int i=0; i<((WORK_STEP_SIZE * WORK_MAX_REQUESTS) - WORK_STEP_SIZE); ++i)
		printf("%u", results[i]);

	printf("\n\nGoodbye.\n");

	return 0;
}
