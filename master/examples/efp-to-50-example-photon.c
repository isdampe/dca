#include <stdbool.h>
#include <stdio.h>
#include "../i2c.h"
#include "../efp.h"

int main()
{
	i2c_obj slave;
	I2C_STATUS status;
	uint8_t main_results[50];
	uint8_t result;
	int i = 0, max = 20, step = 5;
	int mp = 0;

	status = i2c_init(&slave, "/dev/i2c-1", 0x10, I2C_HW_PHOTON);
	if (status != I2C_STATUS_OK)
	{
		printf("Fatal I2C error: %s\n", i2c_get_status_str(status));
		return 1;
	}
	printf("Slave connected\n");

	//Just in case an unclaimed computation exists.
	efp_reset(&slave, 100);

	for (i=0; i<(max -2); ++i)
	{
		result = 0x0;

		printf("Ordering %i: ", i);
		if (efp_order(&slave, i, 500))
			printf("received!\n");
		else {
			printf("timeout!\n");
			i -= 1;
			continue;
		}

		printf("Status wait...\n");
		while (1)
		{
			if (efp_status(&slave, &result, 5000))
				//printf("%i/%i\n", result, step);
			if (result == step)
				break;
			usleep(10000);
		}

		printf("\n");
		
		uint8_t results[step];
		printf("Fetching results: ");
		if (efp_result_range(&slave, results, 1, step, 100))
		{
			for (int x=0; x<step; ++x) {
				printf("%i ", results[x]);
				main_results[mp] = results[x];
				mp++;
			}
			printf("\n");
		} else {
			printf("timeout!\n");
			i -= 1;
			continue;
		}

		printf("Resetting: ");
		if (efp_reset(&slave, 1000))
			printf("done!\n");
		else
			printf("timeout!\n");

	}

	i2c_close(&slave);

	printf("Pi = 3.");
	for (int i=0; i<(max * step); ++i)
		printf("%i", main_results[i]);

	printf("\n");

	return 0;
}
