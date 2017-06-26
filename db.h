#ifndef DB_H
#define DB_H

struct host_s
{
	char *ip;
	char *hostname;
	struct host_s *next;
};

struct host_s *addHost(struct host_s *, char *, char *);

#endif
