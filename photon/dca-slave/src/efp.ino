#include <stdint.h>
#include "efp.h"

void efp_slave_init(efp_slave *slave)
{
	slave->mode = EFP_MODE_IDLE;
	slave->current_job_start_idx = 0x0;
	slave->current_job_progress = 0x0;

	for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
		slave->current_job_results[i] = 0x0;

	for (uint8_t i=0; i<4; ++i)
		slave->registers[i] = 0x0;
}

void efp_slave_parse_registers(const uint32_t reg_val, efp_slave *slave, uint8_t reg_addr)
{
	for (uint8_t i=0x0; i<0x4; ++i)
		slave->registers[i] = (reg_val >> (i * 8) & 0xff);
	
	efp_dump_registers(slave);
}

void efp_dump_registers(const efp_slave *slave)
{
	for (uint8_t i=0x0; i<0x4; ++i)
		Serial.printlnf("Reg %i: 0x%02x", i, slave->registers[i]);
	Serial.printlnf("");
}

uint32_t efp_pack_registers(const efp_slave *slave)
{
	uint32_t result = 0x0;
	
	for (uint8_t i=0x0; i<0x4; ++i)
		result = (result << 8) | slave->registers[i];

	return result;
}