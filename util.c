#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"

void str_tolower(char *str)
{
	char *c;
	while (*(c = str++) != 0x0)
		*c = tolower(*c);
}
