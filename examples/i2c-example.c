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

	printf("Slave connected\n");

	status = i2c_read_reg(&slave);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error: %s\n", i2c_get_status_str(status));
		return 1;
	}
	for (int i=0; i<6; ++i)
	{
		printf("%i: 0x%02x\n", i, slave.reg[i]);
	}


	i2c_set_reg_data(&slave, 1, 0xfa);
	i2c_set_reg_data(&slave, 2, 0xfb);
	i2c_set_reg_data(&slave, 3, 0xfc);
	i2c_set_reg_data(&slave, 4, 0xfd);

	status = i2c_write_reg(&slave);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error: %s\n", i2c_get_status_str(status));
		return 1;
	}
	printf("Wrote to register\n");

	status = i2c_read_reg(&slave);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error: %s\n", i2c_get_status_str(status));
		return 1;
	}
	for (int i=0; i<6; ++i)
	{
		printf("%i: 0x%02x\n", i, slave.reg[i]);
	}

	return 0;
}
