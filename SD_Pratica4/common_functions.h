#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>
#include <stdlib.h>

void checkResult(int result)
{
	if (result == -1) {
		printf("Erro-Close\n\n");
		exit(1);
	}
}

#endif