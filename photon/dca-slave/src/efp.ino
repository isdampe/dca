#include <stdint.h>
#include "efp.h"

//Considering both the system thread and the computation thread access the
//slave struct, mutual exclusion is required to prevent possible corruption
//or deadlock.
static os_mutex_t register_lock;

/**
 * Initialises a given EFP slave to default values.
 * @param slave A pointer to an allocated efp_slave.
 */
void efp_slave_init(efp_slave *slave)
{
	//Create the actual lock only when the slave is intialised.
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

/**
 * Moves each byte of the Photon's 4-byte I2C register into the efp_slave
 * data store struct.
 * @param reg_val  The value of the Photon's 4-byte I2C register
 * @param slave    A pointer to the efp_slave to store bytes in
 */
void efp_slave_parse_registers(const uint32_t reg_val, efp_slave *slave)
{
	os_mutex_lock(register_lock);
	for (uint8_t i=0x0; i<0x4; ++i)
		slave->registers[i] = (reg_val >> (i * 8) & 0xff);
	os_mutex_unlock(register_lock);

	efp_dump_registers(slave);
}

/**
 * Debug function for printing individual register bytes to console.
 * @param slave The efp_slave pointer.
 */
void efp_dump_registers(const efp_slave *slave)
{
	os_mutex_lock(register_lock);
	for (uint8_t i=0x0; i<0x4; ++i)
		Serial.printlnf("Reg %i: 0x%02x", i, slave->registers[i]);
	os_mutex_unlock(register_lock);
	Serial.printlnf("");
}

/**
 * Packs all 4 bytes of the efp_slave's byte array back into a 32-bit
 * integer suitable for writing to the photon's I2C register.
 * @param  slave          A pointer to the efp_spave struct.
 * @return uint32_t       An unsigned 32-bit integer.
 */
uint32_t efp_pack_registers(const efp_slave *slave)
{
	uint32_t result = 0x0;
	os_mutex_lock(register_lock);
	for (int8_t i=0x3; i>=0; --i)
		result = (result << 8) | slave->registers[i];
	os_mutex_unlock(register_lock);
	return result;
}

/**
 * Sets the ACK byte of the efp_slave register.
 * This allows the I2C master to determine the command was read.
 * @param slave A pointer to the efp_slave to be set.
 * @param value The single byte value to write.
 */
void efp_set_ack(efp_slave *slave, const uint8_t value)
{
	os_mutex_lock(register_lock);
	slave->registers[EFP_CMD_REGISTER_SLAVE_ACK_BYTE] = value;
	os_mutex_unlock(register_lock);
}

/**
 * Retrieves a single efp_slave register byte.
 * This method is used to guarantee mutual exclusion.
 * Note: No bounds checking is performed here. It's up to the programmer
 * not to request an out-of-bound index.
 * @param  slave A pointer to the efp_slave
 * @param  index The register byte requested
 * @return       The single byte contained in the register.
 */
uint8_t efp_get_register_byte(const efp_slave *slave, const uint8_t index)
{
	uint8_t result;
	os_mutex_lock(register_lock);
	result = slave->registers[index];
	os_mutex_unlock(register_lock);

	return result;
}

/**
 * Sets a single byte on a specified register for an efp_slave struct.
 * Note: No bounds checking is performed here. It's up to the programmer
 * not to request an out-of-bound index.
 * @param slave A pointer to the efp_slave
 * @param index The register index to update
 * @param val   The single byte value to write
 */
void efp_set_register_byte(efp_slave *slave, const uint8_t index, const uint8_t val)
{
	os_mutex_lock(register_lock);
	slave->registers[index] = val;
	os_mutex_unlock(register_lock);
}

/**
 * Sets the efp_slave's job base. Resets job progress and previous results.
 * Can be safely called to restart a job.
 * @param slave     A pointer to the efp_slave
 * @param start_idx The job sets starting index.
 */
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

/**
 * Set the slave to computation done status.
 * @param slave A pointer to the efp_slave
 */
void efp_set_done(efp_slave *slave)
{
	os_mutex_lock(register_lock);
	slave->mode = EFP_MODE_DONE;
	os_mutex_unlock(register_lock);
}

/**
 * Set the slave to idle status.
 * @param slave A pointer to the efp_slave
 */
void efp_set_idle(efp_slave *slave)
{
	os_mutex_lock(register_lock);
	slave->mode = EFP_MODE_IDLE;
	os_mutex_unlock(register_lock);
}
