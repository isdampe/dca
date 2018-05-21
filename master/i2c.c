#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "i2c.h"

I2C_STATUS i2c_init(i2c_obj *obj, const char *device, const uint32_t addr, const I2C_HW hw_type)
{
	if (strlen(device) > 31)
		return I2C_STATUS_ERR_INVALID_DEVICE_NAME;
	else
		strcpy(obj->device, device);

	obj->addr = addr;
	obj->hw_type = hw_type;
	
	obj->reg[0] = 0x0;
	obj->reg[1] = 0x0;
	obj->reg[2] = 0x0;
	obj->reg[3] = 0x0;
	obj->reg[4] = 0x0;
	obj->reg[5] = 0x0;

	if ((obj->fh = open(obj->device, O_RDWR)) < 0)
		return I2C_STATUS_ERR_OPEN;

	if (ioctl(obj->fh, I2C_SLAVE, obj->addr) < 0)
		return I2C_STATUS_ERR_IOCTL;

	return I2C_STATUS_OK;

}

I2C_STATUS i2c_read_reg(i2c_obj *obj)
{
	//Set register to read to 0x0.
	obj->reg[0] = 0x0;
	obj->reg[1] = 0x0;
	obj->reg[2] = 0x0;
	obj->reg[3] = 0x0;
	obj->reg[4] = 0x0;
	obj->reg[5] = 0x0;

	//Mbed doesn't like the start condition raised.
	if (obj->hw_type != I2C_HW_MBED)
	{
		if (write(obj->fh, obj->reg, 2) != 2)
			return I2C_STATUS_ERR_WRITE_REG;
	}

	if (read(obj->fh, obj->reg, 6) != 6)
		return I2C_STATUS_ERR_READ_REG;

	//for (int i=0; i<6; ++i)
	//	printf("Reg %i: 0x%02x\n\n", i, obj->reg[i]);

	//printf("\n");

	return I2C_STATUS_OK;
}

I2C_STATUS i2c_write_reg(i2c_obj *obj)
{
	//Set register select
	obj->reg[0] = 0x0;
	obj->reg[1] = 0x0;

	//Write the data.
	if (write(obj->fh, obj->reg, 6) != 6)
    	return I2C_STATUS_ERR_WRITE_REG;

	//mBed is slow
	if (obj->hw_type == I2C_HW_MBED)
		usleep(250000); //Update one zero if trouble...

	return I2C_STATUS_OK;
}

I2C_STATUS i2c_set_reg_data(i2c_obj *obj, const uint8_t byte_number, const uint8_t val)
{
	if (byte_number > 4)
		return I2C_STATUS_ERR_REG_OUT_OF_BOUNDS;

	obj->reg[byte_number +1] = val;
	return I2C_STATUS_OK;
}

void i2c_close(i2c_obj *obj)
{
	close(obj->fh);
}

const char *i2c_get_status_str(const I2C_STATUS status)
{
	switch (status)
	{
		case I2C_STATUS_OK:
			return "OK";
			break;
		case I2C_STATUS_ERR_INVALID_DEVICE_NAME:
			return "The device name provided was too long";
			break;
		case I2C_STATUS_ERR_OPEN:
			return "The device name provided was either invalid, not found, or you don't have permission to open it";
			break;
		case I2C_STATUS_ERR_IOCTL:
			return "Could not set slave IO control on device of specified address";
			break;
		case I2C_STATUS_ERR_REG_OUT_OF_BOUNDS:
			return "The register specified was out of bounds";
			break;
		case I2C_STATUS_ERR_READ_REG:
			return "There was an error reading from the specified register";
			break;
		case I2C_STATUS_ERR_WRITE_REG:
			return "There was an error writing to the specified register";
			break;
		default:
			return "Unknown status";
			break;
	}
}
