#include <stdint.h>
#include "efp.h"

void efp_slave_init(efp_slave *slave)
{
	slave->mode = EFP_MODE_IDLE;
	slave->current_job_start_idx = 0x0;
	slave->current_job_progress = 0x0;

	for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
		slave->current_job_results[i] = 0x0;
}