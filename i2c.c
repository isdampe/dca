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

i2c_obj i2c_create(char *device, uint32_t addr)
{
	i2c_obj obj;

	if (strlen(device) > 31)
		perror("Device name is too long");
	else
		strcpy(obj.device, device);

	obj.addr = addr;

	if ((obj.fh = open(obj.device, O_RDWR)) < 0)
		perror("Failed to open device bus.");

	if (ioctl(obj.fh, I2C_SLAVE, obj.addr) < 0)
		perror("Failed to gain bus access to communicate with slave.");

	return obj;

}

uint8_t i2c_read_byte(i2c_obj *obj)
{
	uint8_t result = 0;
	char buf[8] = {0};
	if (read(obj->fh, buf, 8) != 8)
		perror("8 bits were not read.");

	for (int i=0; i<8; ++i)
		printf("The value is %i\n", buf[i]);

	return 0;
}
