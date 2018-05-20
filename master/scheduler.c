#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "scheduler.h"

#define SIG_SLEEP_TIME_MS 50000 //50 milliseconds.

scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule)
{
	scheduler result;
	result.num_workers = num_workers;
	result.current_schedule = 0;
	result.end_schedule = end_schedule;
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

slave *scheduler_get_slave_by_idx(scheduler *s, int8_t idx)
{
	if (idx < s->num_workers)
		return s->slaves[idx];
	return (slave *)NULL;
}

void scheduler_claim_slave(slave *sl)
{
	sl->busy = true;
}

void scheduler_free_slave(slave *sl)
{
	sl->busy = false;
}

void scheduler_destroy(scheduler *s)
{
	for (int8_t i=0; i<s->num_workers; ++i)
		free(s->slaves[i]);
	free(s->slaves);
}
