#include <stdio.h>
#include "../algorithm.h"

int main()
{
	printf("Starting single example...\npi = 3.\n");
	for (int n=1; n<500; ++n)
	{
		printf("%i", get_nth_digit(n));
		if (n % 100 == 0)
			printf("\n");
	}

	printf("\n");

	printf("Starting group example...\npi = 3.\n");
	short results[100];
	for (int i=1; i<=500; i = i + 100)
	{
		get_nth_series(i, i + 99, results);
		for (int k=0; k<100; ++k)
			printf("%i", results[k]);
		printf("\n");
	}

	printf("\n");
	return 0;
}
