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

	printf("Order n=100, i.e. 5*100=500-505...\n");
	if (efp_order(&slave, 6, 100))
		printf("Order received!\n");
	else
		printf("Timeout\n");

	uint8_t result;
	printf("Checking status...\n");
	if (efp_status(&slave, &result, 5000))
		printf("Status is 0x%02x\n", result);
	else
		printf("Timeout");

	usleep(5000000);
	uint8_t results[5];
	printf("Fetching result group, waiting 45 seconds......\n");
	if (efp_result_range(&slave, results, 1, 5, 100))
	{
		for (int i=0; i<5; ++i)
			printf("Result %i: 0x%02x\n", i, results[i]);
	} else
		printf("Timeout fetching results group.\n");

	printf("Resetting...\n");
	if (efp_reset(&slave, 100))
		printf("Reset!\n");
	else
		printf("Timeout resetting\n");

	i2c_close(&slave);

	return 0;
}
