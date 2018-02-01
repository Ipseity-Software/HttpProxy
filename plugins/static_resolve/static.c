#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *resolve(const char *host)
{
	printf("\t--In plugin static, host: %s\n", host);
	if (!strcmp(host, "test2.fake"))
	{
		printf("\t-- -- passing back 2.2.2.2\n");
		return "2.2.2.2";
	}
	printf("\t-- -- passing back NULL\n");
	return NULL;
}
