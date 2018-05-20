#ifndef EFP_H
#define EFP_H
#include <stdint.h>

//EFP - Efficient ........ protocol.
#define EFP_CMD_REGISTER_BYTE 0x1
#define EFP_CMD_REGISTER_SLAVE_ACK_BYTE 0x2
#define EFP_CMD_REGISTER_DATA_BYTE 0x3

#define EFP_SLAVE_ADDR 0x10
#define EFP_SLAVE_REGISTERS 0x2

#define EFP_JOB_FACTOR 0x10

typedef enum
{
	EFP_CMD_PING = 0x0,
	EFP_CMD_ORDER = 0x1,
	EFP_CMD_STATUS = 0x2,
	EFP_CMD_RESULT = 0x3,
	EFP_CMD_CANCEL = 0x4
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
} efp_slave;

void efp_slave_init(efp_slave *slave);

#endif
