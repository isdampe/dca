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
	EFP_CMD_PING = 0x0,
	EFP_CMD_ORDER = 0x1,
	EFP_CMD_STATUS = 0x2,
	EFP_CMD_RESULT = 0x3,
	EFP_CMD_CANCEL = 0x4
} EFP_CMD;

static bool efp_wait_ack(i2c_obj *obj, const uint32_t timeout_ns);
bool efp_ping(i2c_obj *obj, const uint32_t timeout_ms);
bool efp_order(i2c_obj *obj, const uint8_t n_val, const uint32_t timeout_ms);
bool efp_status(i2c_obj *obj, uint8_t *des, const uint32_t timeout_ms);
bool efp_result_single(i2c_obj *obj, uint8_t *des, const uint8_t req_idx, const uint32_t timeout_ms);
bool efp_result_range(i2c_obj *obj, uint8_t *des, uint8_t start_idx, const uint8_t end_idx, const uint32_t timeout_ms);
bool efp_cancel(i2c_obj *obj, const uint32_t timeout_ms);
#endif
