#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "efp.h"

static bool efp_wait_ack(i2c_obj *obj, const uint32_t timeout_ns)
{
	struct timespec time_begin, time_current;

	clock_gettime(CLOCK_REALTIME, &time_begin);

	//Busy wait for a response.
	while (obj->reg[EFP_CMD_REGISTER_SLAVE_ACK_BYTE -1] == 0x0)
	{
		i2c_read_reg(obj);

		clock_gettime(CLOCK_REALTIME, &time_current);
		if ((time_current.tv_nsec - time_begin.tv_nsec) > timeout_ns)
			return false;
	}

	return true;
}

bool efp_ping(i2c_obj *obj, const uint32_t timeout_ms)
{
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_BYTE, EFP_CMD_PING);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_SLAVE_ACK_BYTE, 0x0);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_DATA_BYTE, 0x0);

	if (i2c_write_reg(obj) != I2C_STATUS_OK)
		return false;

	return efp_wait_ack(obj, timeout_ms * 1000000);
}

bool efp_order(i2c_obj *obj, const uint8_t n_val, const uint32_t timeout_ms)
{
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_BYTE, EFP_CMD_ORDER);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_SLAVE_ACK_BYTE, 0x0);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_DATA_BYTE, n_val);

	if (i2c_write_reg(obj) != I2C_STATUS_OK)
		return false;

	return efp_wait_ack(obj, timeout_ms * 1000000);
}

bool efp_status(i2c_obj *obj, uint8_t *des, const uint32_t timeout_ms)
{
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_BYTE, EFP_CMD_STATUS);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_SLAVE_ACK_BYTE, 0x0);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_DATA_BYTE, 0x0);

	if (i2c_write_reg(obj) != I2C_STATUS_OK)
		return false;

	if (! efp_wait_ack(obj, timeout_ms * 1000000))
		return false;

	*des = obj->reg[EFP_CMD_REGISTER_DATA_BYTE -1];
	return true;
}

bool efp_result_single(i2c_obj *obj, uint8_t *des, const uint8_t req_idx, const uint32_t timeout_ms)
{
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_BYTE, EFP_CMD_RESULT);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_SLAVE_ACK_BYTE, 0x0);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_DATA_BYTE, req_idx);

	if (i2c_write_reg(obj) != I2C_STATUS_OK)
		return false;

	if (! efp_wait_ack(obj, timeout_ms * 1000000))
		return false;

	*des = obj->reg[EFP_CMD_REGISTER_DATA_BYTE -1];
	return true;
}

bool efp_result_range(i2c_obj *obj, uint8_t *des, uint8_t start_idx, const uint8_t end_idx, const uint32_t timeout_ms)
{
	uint8_t i = 0;
	do
	{
		if (! efp_result_single(obj, &des[i], start_idx, timeout_ms))
			return false;
		++i;
	} while (++start_idx <= end_idx);
}