#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "efp.h"

bool efp_ping(i2c_obj *obj, const uint32_t timeout)
{
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_BYTE, EFP_CMD_PING);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_SLAVE_ACK_BYTE, 0x0);
	i2c_set_reg_data(obj, EFP_CMD_REGISTER_DATA_BYTE, 0x0);

	if (i2c_write_reg(obj) != I2C_STATUS_OK)
		return false;

	//Busy wait for a response.
	while (obj->reg[EFP_CMD_REGISTER_SLAVE_ACK_BYTE -1] == 0x0)
	{
		i2c_read_reg(obj);
		for (int i=0; i<6; ++i)
		{
			printf("%i: 0x%02x\n", i, obj->reg[i]);
		}
		//Put a timeout constraint here.
	}

	return true;

}
