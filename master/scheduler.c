#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "scheduler.h"
#include "i2c.h"

#define SIG_SLEEP_TIME_MS 50000 //50 milliseconds.

/**
 * Creates a new scheduler and initialises the values.
 * @param  num_workers  The number of workers in the scheduler
 * @param  end_schedule The job end number (not used).
 * @return              A new scheduler instance structure.
 */
scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule)
{
	scheduler result;
	result.num_workers = num_workers;
	result.current_schedule = 0;
	result.slaves = calloc(num_workers, sizeof(slave *));

	for (int i=0; i<num_workers; ++i)
	{
		result.slaves[i] = calloc(1, sizeof(slave));
		result.slaves[i]->idx = i;
		result.slaves[i]->addr = i;
		result.slaves[i]->busy = false;
	}

	return result;
}

/**
 * Sets i2c_obj data for a given scheduler.
 * @param s    A pointer to the scheduler.
 * @param idx  The slave index to set.
 * @param obj  A pointer to the i2c_obj for association.
 * @param name A string name of the slave
 */
void scheduler_set_slave_i2c(scheduler *s, const uint8_t idx, i2c_obj *obj, char *name)
{
	s->slaves[idx]->obj = obj;
	s->slaves[idx]->name = name;
}

/**
 * Finds a slave that is doing nothing.
 * @param  s          A pointer to the scheduler.
 * @param  timeout_ms A timeout upper bound limiting the spinlock behaviour
 * @return            -1 if no slaves are available, otherwise the index number of the slave.
 */
int8_t scheduler_get_free_slave_idx(scheduler *s, uint32_t timeout_ms)
{
	uint32_t rough_time_passed = 0;
	timeout_ms = timeout_ms * 1000;

	//Fix me. Add timeout.
	while (rough_time_passed < timeout_ms)
	{
		for (int8_t i=0; i<s->num_workers; ++i)
			if (! s->slaves[i]->busy)
				return i;
		usleep(SIG_SLEEP_TIME_MS);
		rough_time_passed += SIG_SLEEP_TIME_MS;
	}

	return -1;
}

/**
 * Returns a pointer to the slave for a given index.
 * @param  s   A pointer to the scheduler
 * @param  idx The index of the slave to retrieve
 * @return     A pointer to the slave if it exists. Otherwise, NULL.
 */
slave *scheduler_get_slave_by_idx(scheduler *s, int8_t idx)
{
	if (idx < s->num_workers)
		return s->slaves[idx];
	return (slave *)NULL;
}

/**
 * Claim a slave for work.
 * @param sl A pointer to the slave.
 */
void scheduler_claim_slave(slave *sl)
{
	sl->busy = true;
}

/**
 * Free a slave for accepting more work.
 * @param sl A pointer to the slave.
 */
void scheduler_free_slave(slave *sl)
{
	sl->busy = false;
}

/**
 * Destroys a scheduler instance.
 * @param s A pointer to the scheduler to destroy.
 */
void scheduler_destroy(scheduler *s)
{
	for (int8_t i=0; i<s->num_workers; ++i)
		free(s->slaves[i]);
	free(s->slaves);
}
