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

I2C_STATUS i2c_init(i2c_obj *obj, const char *device, const uint32_t addr)
{
	if (strlen(device) > 31)
		return I2C_STATUS_ERR_INVALID_DEVICE_NAME;
	else
		strcpy(obj->device, device);

	obj->addr = addr;

	if ((obj->fh = open(obj->device, O_RDWR)) < 0)
		return I2C_STATUS_ERR_OPEN;

	if (ioctl(obj->fh, I2C_SLAVE, obj->addr) < 0)
		return I2C_STATUS_ERR_IOCTL;

	return I2C_STATUS_OK;

}

uint8_t i2c_read_byte(const i2c_obj *obj, const uint8_t register_num)
{
	uint8_t result = 0;
	char buf[8] = {0};
	if (read(obj->fh, buf, 8) != 8)
		perror("8 bits were not read.");

	for (int i=0; i<8; ++i)
		printf("The value is %i\n", buf[i]);

	return 0;
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
		default:
			return "Unknown status";
			break;
	}
}
