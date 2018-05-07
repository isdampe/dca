#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t idx;
	uint8_t addr;	
	bool busy;
} slave;

typedef struct {
	uint8_t num_workers;
	slave *slaves;
	uint32_t current_schedule;
	uint32_t end_schedule;
} scheduler;

scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule);
void scheduler_destroy(scheduler *s);
slave *scheduler_get_free_slave(scheduler *s, uint32_t timeout);
void scheduler_claim_slave(slave *sl);

#endif
