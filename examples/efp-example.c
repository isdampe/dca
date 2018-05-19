#include <stdbool.h>
#include <stdio.h>
#include "../i2c.h"
#include "../efp.h"

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

	printf("Ping...\n");
	if (efp_ping(&slave, 100)) //100ms timeout.
		printf("Ping received!\n");
	else
		printf("Timeout\n");

	printf("Order n=1, i.e. 5*1=1-5...\n");
	if (efp_order(&slave, 1, 100))
		printf("Order received!\n");
	else
		printf("Timeout\n");

	return 0;
}
