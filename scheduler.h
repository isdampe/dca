#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t idx;
	uint8_t addr;	
	volatile bool busy;
} slave;

typedef struct {
	uint8_t num_workers;
	slave **slaves;
	uint32_t current_schedule;
	uint32_t end_schedule;
} scheduler;

scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule);
int8_t scheduler_get_free_slave_idx(scheduler *s, uint32_t timeout_ms);
slave *scheduler_get_slave_by_idx(scheduler *s, int8_t idx);
void scheduler_claim_slave(slave *sl);
void scheduler_free_slave(slave *sl);
void scheduler_destroy(scheduler *s);

#endif
