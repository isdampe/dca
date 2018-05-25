#ifndef EFP_H
#define EFP_H
#include <stdint.h>

//EFP - Efficient ........ protocol.
#define EFP_CMD_REGISTER_BYTE 0x0
#define EFP_CMD_REGISTER_SLAVE_ACK_BYTE 0x1
#define EFP_CMD_REGISTER_DATA_BYTE 0x2

#define EFP_SLAVE_ADDR 0x10
#define EFP_SLAVE_REGISTERS 0x2

#define EFP_JOB_FACTOR 0x5

#define EFP_ACK_OK 0x1
#define EFP_ACK_ERR 0x2

typedef enum
{
	EFP_CMD_PING = 0x0,
	EFP_CMD_ORDER = 0x1,
	EFP_CMD_STATUS = 0x2,
	EFP_CMD_RESULT = 0x3,
	EFP_CMD_RESET = 0x4
} EFP_CMD;

typedef enum
{
	EFP_MODE_IDLE,
	EFP_MODE_WORK,
	EFP_MODE_DONE
} EFP_MODE;

typedef struct
{
	EFP_MODE mode;
	uint8_t current_job_start_idx;
	uint8_t current_job_progress = 0x0;
	uint8_t current_job_results[EFP_JOB_FACTOR];
	uint16_t reg_val;
	uint8_t registers[4];
} efp_slave;

void efp_slave_init(efp_slave *slave);
void efp_slave_parse_registers(const uint32_t reg_val, efp_slave *slave);
void efp_dump_registers(const efp_slave *slave);
uint32_t efp_pack_registers(const efp_slave *slave);
void efp_set_ack(efp_slave *slave, const uint8_t value);
uint8_t efp_get_register_byte(const efp_slave *slave, const uint8_t index);
void efp_set_register_byte(efp_slave *slave, const uint8_t index, const uint8_t val);
void efp_set_job(efp_slave *slave, const uint8_t start_idx);
void efp_set_done(efp_slave *slave);
void efp_set_idle(efp_slave *slave);

#endif
