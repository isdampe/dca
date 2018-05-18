#ifndef EFP_H
#define EFP_H
#include <stdbool.h>
#include <stdint.h>
#include "i2c.h"

//EFP - Efficient ........ protocol.

#define EFP_CMD_REGISTER_BYTE 0x1
#define EFP_CMD_REGISTER_SLAVE_ACK_BYTE 0x2
#define EFP_CMD_REGISTER_DATA_BYTE 0x3


typedef enum
{
	EFP_CMD_PING,
	EFP_CMD_ORDER,
	EFP_CMD_STATUS,
	EFP_CMD_RESULT,
	EFP_CMD_CANCEL
} EFP_CMD;

bool efp_ping(i2c_obj *obj, const uint32_t timeout);

#endif
