#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"

typedef struct {
	uint8_t idx;
	uint8_t addr;
	//When gcc uses -O1 or higher optimsiations, this gets optimised out
	//and deadlock can occur. MUST be defined volatile.
	volatile bool busy;
	i2c_obj *obj;
	char *name;
	uint32_t current_idx;
} slave;

typedef struct {
	uint8_t num_workers;
	slave **slaves;
	uint32_t current_schedule;
	uint32_t end_schedule;
} scheduler;

scheduler scheduler_create(const uint8_t num_workers, const uint32_t end_schedule);
void scheduler_set_slave_i2c(scheduler *s, const uint8_t idx, i2c_obj *obj, char *name);
int8_t scheduler_get_free_slave_idx(scheduler *s, uint32_t timeout_ms);
slave *scheduler_get_slave_by_idx(scheduler *s, int8_t idx);
void scheduler_claim_slave(slave *sl);
void scheduler_free_slave(slave *sl);
void scheduler_destroy(scheduler *s);

#endif
