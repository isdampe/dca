#include <stdint.h>
#include "efp.h"

static os_mutex_t register_lock;

void efp_slave_init(efp_slave *slave)
{
	os_mutex_create(&register_lock);

	os_mutex_lock(register_lock);
	slave->mode = EFP_MODE_IDLE;
	slave->current_job_start_idx = 0x0;
	slave->current_job_progress = 0x0;

	for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
		slave->current_job_results[i] = 0x0;

	for (uint8_t i=0; i<4; ++i)
		slave->registers[i] = 0x0;

	os_mutex_unlock(register_lock);
}

void efp_slave_parse_registers(const uint32_t reg_val, efp_slave *slave, uint8_t reg_addr)
{
	os_mutex_lock(register_lock);
	for (uint8_t i=0x0; i<0x4; ++i)
		slave->registers[i] = (reg_val >> (i * 8) & 0xff);
	os_mutex_unlock(register_lock);
	
	efp_dump_registers(slave);
}

void efp_dump_registers(const efp_slave *slave)
{
	os_mutex_lock(register_lock);
	for (uint8_t i=0x0; i<0x4; ++i)
		Serial.printlnf("Reg %i: 0x%02x", i, slave->registers[i]);
	os_mutex_unlock(register_lock);
	Serial.printlnf("");
}

uint32_t efp_pack_registers(const efp_slave *slave)
{
	uint32_t result = 0x0;
	os_mutex_lock(register_lock);
	for (int8_t i=0x3; i>=0; --i)
		result = (result << 8) | slave->registers[i];
	os_mutex_unlock(register_lock);
	return result;
}

void efp_set_ack(efp_slave *slave, const uint8_t value)
{
	os_mutex_lock(register_lock);
	slave->registers[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = value;
	os_mutex_unlock(register_lock);
}

uint8_t efp_get_register_byte(const efp_slave *slave, const uint8_t index)
{
	uint8_t result;
	os_mutex_lock(register_lock);
	result = slave->registers[index];
	os_mutex_unlock(register_lock);

	return result;
}

void efp_set_register_byte(efp_slave *slave, const uint8_t index, const uint8_t val)
{
	os_mutex_lock(register_lock);
	slave->registers[index] = val;
	os_mutex_unlock(register_lock);
}

void efp_set_job(efp_slave *slave, const uint8_t start_idx)
{
	os_mutex_lock(register_lock);
	slave->current_job_start_idx = start_idx;
	slave->current_job_progress = 0x0;
	for (uint8_t i=0; i<EFP_JOB_FACTOR; ++i)
		slave->current_job_results[i] = 0x0;
	slave->mode = EFP_MODE_WORK;
	os_mutex_unlock(register_lock);
}

void efp_set_done(efp_slave *slave)
{
	os_mutex_lock(register_lock);
	slave->mode = EFP_MODE_DONE;
	os_mutex_unlock(register_lock);
}

void efp_set_idle(efp_slave *slave)
{
	os_mutex_lock(register_lock);
	slave->mode = EFP_MODE_IDLE;
	os_mutex_unlock(register_lock);
}