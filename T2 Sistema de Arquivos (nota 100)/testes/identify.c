#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"

int main(){
	char *identify = malloc(75);	

	identify = t2fs_identify();

	printf("%s\n", identify);
	return 0;
}
