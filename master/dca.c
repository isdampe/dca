#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "scheduler.h"
#include "i2c.h"
#include "efp.h"

#define WORK_STEP_SIZE 5
#define WORK_MAX_REQUESTS 30
#define EFP_ORDER_TIMEOUT 500

static i2c_obj slave_photon, slave_mbed;
static I2C_STATUS status;
static scheduler s;
static uint8_t results[WORK_STEP_SIZE * WORK_MAX_REQUESTS];
static uint8_t jobs[WORK_MAX_REQUESTS];

void setup_jobs()
{
	for (int i=0; i<WORK_MAX_REQUESTS; ++i)
		jobs[i] = 0x0;
}

bool setup_i2c_slaves()
{
	status = i2c_init(&slave_photon, "/dev/i2c-1", 0x10, I2C_HW_PHOTON);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error on photon: %s\n", i2c_get_status_str(status));
		return false;
	}

	status = i2c_init(&slave_mbed, "/dev/i2c-1", 0x50, I2C_HW_MBED);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error on mbed: %s\n", i2c_get_status_str(status));
		return false;
	}

	return true;
}

bool setup_scheduler()
{
	s = scheduler_create(2, 25);
	scheduler_set_slave_i2c(&s, 0, &slave_photon, "photon");
	scheduler_set_slave_i2c(&s, 1, &slave_mbed, "mbed");

	return true;
}

int job_get_next()
{
	for (int i=0; i<WORK_MAX_REQUESTS; ++i)
	{
		if (jobs[i] == 0x0)
			return i;
	}

	return -1;
}

void auto_dispatch_work()
{
	int8_t current_slave = scheduler_get_free_slave_idx(&s, 500);
	slave *sl;

	int current_job = job_get_next();

	if (current_slave >= 0)
	{
		sl = scheduler_get_slave_by_idx(&s, current_slave);
		scheduler_claim_slave(sl);

		//Reset first.
		efp_reset(sl->obj, 100);

		//Create work order.
		if (! efp_order(sl->obj, current_job, EFP_ORDER_TIMEOUT))
		{
			printf("Timeout ordering %s to compute from %i\n", sl->name, current_job);
			scheduler_free_slave(sl);
			return;
		}

		sl->current_idx = current_job;

		printf("Ordered %s to compute from start index %i\n", sl->name, current_job);
		s.current_schedule++;
		jobs[current_job] = 0x1;
	}
}

void check_results()
{
	for (int8_t i=0; i<s.num_workers; ++i)
	{
		if (! s.slaves[i]->busy)
			continue;

		uint8_t result;

		if (efp_status(s.slaves[i]->obj, &result, 5000) && result == WORK_STEP_SIZE)
		{
			printf("The %s has finished\n", s.slaves[i]->name);

			uint8_t step_results[WORK_STEP_SIZE];
			if (efp_result_range(s.slaves[i]->obj, step_results, 1, WORK_STEP_SIZE, 100))
			{
				uint32_t idx = (s.slaves[i]->current_idx * WORK_STEP_SIZE);
				for (uint8_t x=0; x<WORK_STEP_SIZE; ++x)
					results[idx + x] = step_results[x];

				//Free up the slave.
				efp_reset(s.slaves[i]->obj, 100);
				scheduler_free_slave(s.slaves[i]);
			}
			else 
			{
				printf("An error occured fetching results from %s. Releasing to queue\n", s.slaves[i]->name);
				jobs[s.slaves[i]->current_idx] = 0x0;
				efp_reset(s.slaves[i]->obj, 100);
				scheduler_free_slave(s.slaves[i]);
			}

			printf("Status: %i / %i\n", s.current_schedule, WORK_MAX_REQUESTS);
		}
		usleep(10000);

	}

}

int main(int argc, char **argv)
{
	printf("Setting up jobs...\n");
	setup_jobs();
	
	printf("Setting up I2C devices...\n");
	if (! setup_i2c_slaves())
		return 1;

	printf("Creating scheduler...\n");
	if (! setup_scheduler())
		return 1;

	while (job_get_next() > -1)
	{
		auto_dispatch_work();
		check_results();
	}

	printf("Computation complete\n\n");
	printf("pi = 3.");
	for (int i=0; i<(WORK_STEP_SIZE * WORK_MAX_REQUESTS); ++i)
		printf("%u", results[i]);

	printf("\n\nGoodbye.\n");

	return 0;
}