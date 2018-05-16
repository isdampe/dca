#include <stdio.h>
#include "../i2c.h"

int main()
{
	i2c_obj slave = i2c_create("/dev/i2c-1", 0x10);
	uint8_t byte = i2c_read_byte(&slave);
	printf("Slave connected\n");
	return 0;
}
