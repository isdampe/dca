#include <stdio.h>
#include "../i2c.h"

int main()
{
	i2c_obj slave;
	I2C_STATUS status;
	
	status = i2c_init(&slave, "/dev/i2c-1", 0x10);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error: %s\n", i2c_get_status_str(status));
		return 1;
	}

	uint8_t byte = i2c_read_byte(&slave, 0x1); //Read one byte from register 1.
	printf("Slave connected\n");
	return 0;
}
