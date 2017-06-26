/* general headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
/* misc libs */
#include <string.h>
#include <time.h>
/* project libs */
#include <db.h>
#include <util.h>



struct host_s *addHost(struct host_s * lst, char * hostname, char * ip)
{
	struct host_s *node;
	struct host_s *ptr;
	node = malloc(sizeof(struct host_s));
	node->hostname = strdup(hostname);
	node->ip = strdup(ip);
	node->next = NULL;
	if (lst == NULL)
		return node;
	for(ptr = lst; ptr->next != NULL; ptr = ptr->next);
	ptr->next = node;
	return lst;
}
