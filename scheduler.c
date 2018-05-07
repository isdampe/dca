#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "scheduler.h"

scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule)
{
	scheduler result;
	result.num_workers = num_workers;
	result.current_schedule = 0;
	result.end_schedule = end_schedule;
	result.slaves = calloc(num_workers, sizeof(slave));
	
	for (int i=0; i<num_workers; ++i)
	{
		result.slaves[i].idx = i;
		result.slaves[i].addr = i;
		result.slaves[i].busy = false;
	}

	return result;
}

void scheduler_destroy(scheduler *s)
{
	free(s->slaves);
}

slave *scheduler_get_free_slave(scheduler *s, uint32_t timeout)
{
	//Fix me. Add timeout.
	while (1)
	{
		for (int i=0; i<s->num_workers; ++i)
			if (! s->slaves[i].busy)
				return &s->slaves[i];
		usleep(50000); //50 milliseconds.
	}
}

void scheduler_claim_slave(slave *sl)
{
	//Mutual exclusion here?
	sl->busy = true;
}
