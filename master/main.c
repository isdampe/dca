#include <stdio.h>
#include "dca.h"

int main(int argc, char **argv)
{
	char c, res;

	while (c != '2')
	{
		printf("What would you like to do?\n1. Run computation\n2. Quit\n");
		scanf("%c", &c);
		if (c == '1')
			res = dca_main();
	}
	return res;
}