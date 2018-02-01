#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *resolve(const char *host)
{
	printf("\t--In plugin static.resolve, host: %s\n", host);
	if (!strcmp(host, "test2.fake"))
		return "2.2.2.2";
	return NULL;
}
