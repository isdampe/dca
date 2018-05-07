#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../scheduler.h"

int main()
{
	slave *sl;

	printf("Creating scheduler...\n");
	scheduler s = scheduler_create(2, 10);

	for (int x=0; x<4; x++) {
		printf("Finding slave...\n");
		int8_t current_slave;
		current_slave = scheduler_get_free_slave_idx(&s, 1000);
		if (current_slave >= 0) {
			printf("Using slave index %i\n", current_slave);
			printf("Claiming slave...\n");
			sl = scheduler_get_slave_by_idx(&s, current_slave);
			scheduler_claim_slave(sl);
		} else {
			printf("Timeout getting slave...\n");
			if (sl != NULL) {
				printf("Releasing a slave...\n");
				scheduler_free_slave(sl);
			}
		}
	}

	printf("Destroying scheduler...\n");
	scheduler_destroy(&s);
	return 0;
}
