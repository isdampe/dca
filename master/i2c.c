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

/**
 * Initialises an i2c object.
 * @param  obj     A pointer to the i2c_obj struct.
 * @param  device  The name of the device.
 * @param  addr    The I2C hardware address of the device (8-bit).
 * @param  hw_type The hardware type of the device. Must be set.
 * @return         An I2C_STATUS code.
 */
I2C_STATUS i2c_init(i2c_obj *obj, const char *device, const uint32_t addr, const I2C_HW hw_type)
{
	//We only have 32 bytes to store the device name.
	if (strlen(device) > 31)
		return I2C_STATUS_ERR_INVALID_DEVICE_NAME;
	else
		strcpy(obj->device, device);

	obj->addr = addr;
	obj->hw_type = hw_type;

	//Reset the registers.
	obj->reg[0] = 0x0;
	obj->reg[1] = 0x0;
	obj->reg[2] = 0x0;
	obj->reg[3] = 0x0;
	obj->reg[4] = 0x0;
	obj->reg[5] = 0x0;

	//Request device access from the kernel.
	if ((obj->fh = open(obj->device, O_RDWR)) < 0)
		return I2C_STATUS_ERR_OPEN;

	//Set I2C slave mode on the file descriptor.
	if (ioctl(obj->fh, I2C_SLAVE, obj->addr) < 0)
		return I2C_STATUS_ERR_IOCTL;

	return I2C_STATUS_OK;

}

/**
 * Reads the registers of a given slave i2c_obj.
 * @param  obj A pointer to the i2c_obj.
 * @return     An I2C_STATUS code.
 */
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
	//This could be clock-speed related or a bug in the I2C slave
	//driver for Mbed.
	if (obj->hw_type != I2C_HW_MBED)
	{
		if (write(obj->fh, obj->reg, 2) != 2)
			return I2C_STATUS_ERR_WRITE_REG;
	}
	if (read(obj->fh, obj->reg, 6) != 6)
		return I2C_STATUS_ERR_READ_REG;

	return I2C_STATUS_OK;
}

/**
 * Writes the values of an i2c_obj register to the slaves registers.
 * @param  obj A pointer to the i2c_obj
 * @return     An I2C_STATUS code.
 */
I2C_STATUS i2c_write_reg(i2c_obj *obj)
{
	//Set register select
	obj->reg[0] = 0x0;
	obj->reg[1] = 0x0;

	//Write the data.
	if (write(obj->fh, obj->reg, 6) != 6)
    	return I2C_STATUS_ERR_WRITE_REG;

	//Mbed is unreliable in testing without giving it time to process.
	//Again, could be clock-speed related...
	if (obj->hw_type == I2C_HW_MBED)
		usleep(250000);

	return I2C_STATUS_OK;
}

/**
 * Sets a given register byte number with a given value on an i2c_obj.
 * @param  obj         A pointer to the i2c_obj.
 * @param  byte_number The byte number to set in the register.
 * @param  val         The 8-bit value to set.
 * @return             An I2C_STATUS code.
 */
I2C_STATUS i2c_set_reg_data(i2c_obj *obj, const uint8_t byte_number, const uint8_t val)
{
	if (byte_number > 4)
		return I2C_STATUS_ERR_REG_OUT_OF_BOUNDS;

	obj->reg[byte_number +1] = val;
	return I2C_STATUS_OK;
}

/**
 * Closes off the I2C connection from kernel.
 * @param obj A pointer to the i2c_obj.
 */
void i2c_close(i2c_obj *obj)
{
	close(obj->fh);
}

/**
 * Converts a given I2C_STATUS value to a user-friendly string of characters.
 * @param  status The I2C_STATUS code.
 * @return        A readable string of characters.
 */
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

/**
 * Converts a given i2c_obj's registers to a string of characters in hexidecimal.
 * @param obj  A pointer to the i2c_obj.
 * @param dest The location to store the string of characters.
 */
void i2c_reg_to_string(const i2c_obj *obj, char *dest)
{
	char buffer[61]; char sbuffer[5];
	buffer[0] = '\0';
	sbuffer[0] = '\0';

	for (uint8_t i=0; i<6; sprintf(sbuffer, "%02x ", obj->reg[i++]))
		strcat(buffer, sbuffer);
	strcat(dest, buffer);

}
