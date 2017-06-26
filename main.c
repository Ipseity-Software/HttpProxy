/* general headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>
/* misc libs */
#include <string.h>
#include <time.h>
#include <pthread.h>
/* socket related */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
/* project libs */
#include <util.h>
#include <db.h>

#define BUFSIZE 4096

struct list_s
{
	pthread_t tid;
	int fd;
	bool complete;
	struct list_s *next;
};


struct host_s *hosts;

struct list_s *addThread(struct list_s *lst, struct list_s *node, pthread_t tid)
{
	struct list_s *tmp;
	if (node == NULL)
		return lst;
	node->tid = tid;
	if (lst == NULL)
		return node;
	for (tmp = lst; tmp->next != NULL; tmp = tmp->next);
	tmp->next = node;
	return lst;
}

struct list_s *createThread(int fd)
{
	struct list_s *node;
	node = malloc(sizeof(struct list_s));
	node->tid = 0;
	node->complete = false;
	node->next = NULL;
	node->fd = fd;
	return node;
}

struct list_s *removeThread(struct list_s *lst, pthread_t tid)
{
	struct list_s *node;
	struct list_s *pnode;
	for (node = lst, pnode = lst; node->tid != tid && node != NULL; pnode = node, node = node->next);
	if (node == NULL)
		return lst;
	if (node == pnode)
		lst = node->next;
	pnode->next = node->next;
	free(node);
	return lst;
}

char *split_header(char *buf, char *lastbyte, char **next)
{
	char *ret;
	char *tmp;
	bool f1;
	int i;
	if (buf == NULL || next == NULL)
		return NULL;
	if (lastbyte == buf)
		return NULL;
	for (i = 0, f1 = false, tmp = buf; tmp != lastbyte; ++tmp, ++i)
	{
		if (!f1)
		{
			if (*tmp == '\r')
				f1 = true;
		}
		else
		{
			if (*tmp == '\n')
			{
				--i; //make i length to '\r'
				--tmp; // make tmp point at '\r'
				*next = tmp + 2; // next line is 2 bytes forward
				break;
			}
			else
				f1 = false;
		}
	}
	ret = malloc(i + 1);
	strncpy(ret, buf, i);
	ret[i] = 0x0; //null term at end
	return ret;
}

char *find_host(char *data, ssize_t len)
{
	char *scratch;
	char *ptr;
	char *lastbyte;
	char *line;
	char *ret;
	scratch = malloc(len + 1);
	strncpy(scratch, data, len);
	scratch[len] = 0x00;
	ptr = scratch;
	lastbyte = &scratch[len];
	ret = NULL;

	while ((line = split_header(ptr, lastbyte, &ptr)) != NULL)
	{
		if (strstr(line, "Host: ") == line) //starts with "HOST: "
		{
			ret = malloc(strlen(line));
			memset(ret, 0x00, strlen(line));
			strncpy(ret, (const char *)(line + strlen("HOST: ")), strlen(line) - 1);
			str_tolower(ret);
			printf("[Decode] Host is %s\n", ret);
			free(line);
			goto done;
		}
		free(line);
	}
done:
	ptr = NULL;
	lastbyte = NULL;
	free(scratch);
	scratch = NULL;
	line = NULL;
	return ret;
}

char *determine_request(char *buffer, ssize_t len)
{
	char *ip;
	char *host;
	char *addname;
	char *addip;
	uint8_t tmp[4];
	struct host_s *ptr;
	ip = NULL;
	host = find_host(buffer, len);
	printf("Debug: %s\n", host);
	if (strstr(host, "add.") != NULL) // starts with addhost directive
	{
		printf("Host has add. in it\n");
		addname = malloc(768);
		addip = malloc(32);
		sscanf(host, "add.%[^_]_%d.%d.%d.%d.ipseitysoftware.com", addname, &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
		sprintf(addip, "%u.%u.%u.%u", tmp[0], tmp[1], tmp[2], tmp[3]);
		printf("Adding %s -> %s\n", addname, addip);
		hosts = addHost(hosts, addname, addip);
		return strdup(addip);
	}
	for (ptr = hosts; ptr != NULL && strcmp(ptr->hostname, host); ptr = ptr->next);
	if (ptr)
		ip = strdup(ptr->ip);
	return ip;
}

int proxy(const char *ip, char *buffer, ssize_t len, int fd, uint16_t port)
{
	char *recvbuf; // from the server
	int sockfd; // server
	struct sockaddr_in serv_addr;
	ssize_t nwritten;
	ssize_t nread;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 1;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
		return 2;
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		return -1; // connection refused
	recvbuf = malloc(BUFSIZE);
	memset(recvbuf, 0x0, BUFSIZE);
	nwritten = write(sockfd, buffer, len);
	printf("Sending request\n");
	// check if data left on input pipe
	while (poll(&(struct pollfd) { .fd = fd, .events = POLLIN }, 1, 100) == 1)
	{
		nread = read(fd, buffer, BUFSIZE);
		if (nread == 0)
			break;
		nwritten = write(sockfd, buffer, nread);
		printf("Read extra from client:\tIN: %d\tOUT: %d\n", nread, nwritten);
	}
	printf("Processing initial response\n");
	nread = read(sockfd, recvbuf, BUFSIZE);
	nwritten = write(fd, recvbuf, nread);
	// check for data left on output pipeline
	while (poll(&(struct pollfd) { .fd = sockfd, .events = POLLIN }, 1, 100) == 1)
	{
		nread = read(sockfd, recvbuf, BUFSIZE);
		if (nread == 0)
			break;
		nwritten = write(fd, recvbuf, nread);
		printf("Read extra from server:\tIN: %d\tOUT: %d\n", nread, nwritten);
	}
	// all finished, shutdown
	shutdown(sockfd, 2);
	shutdown(fd, 2);
	printf("Connection closed\n");
	free(recvbuf);
	return 0;
}

void *process_request(void *param)
{
	struct list_s *node;
	char *buffer;
	char *hostip;
	ssize_t nread;
	int retval;
	uint16_t hostport;
	char *_404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	char *_503 = "HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	node = param;
	buffer = malloc(4096); // allocate input buffer
	nread = read(node->fd, buffer, BUFSIZE);
	if (nread == 0) // Early termination
	{
		node->complete = true;
		return node;
	}
	else // we got real data
	{
		hostip = determine_request(buffer, nread);
		hostport = 80;
		if (hostip != NULL)
		{
			retval = proxy(hostip, buffer, nread, node->fd, hostport);
			if (retval == -1)
			{
				printf("Host refused connection!\n");
				write(node->fd, _503, strlen(_503));
				shutdown(node->fd, 3);
			}
			free(hostip);
		}
		else
		{
			printf("Host not found!\n");
			write(node->fd, _404, strlen(_404));
			shutdown(node->fd, 3);
		}
	}
}


int main(int argc, char ** argv)
{
	/* declarations */
	int listenfd;
	int connfd;
	struct sockaddr_in serv_addr;
	struct list_s *threads;
	struct list_s *node;
	pthread_t tid;
	/* init */
	listenfd = 0;
	connfd = 0;
	threads = NULL;
	hosts = addHost(hosts, "www.ipseitysoftware.com", "10.1.0.19");
	/* */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(80);

	bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	printf("Ready\n");
	for (;;)
	{
		connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		node = createThread(connfd);
		tid = pthread_create(&tid, NULL, process_request, node);
		threads = addThread(threads, node, tid);
		printf("Accepted client\n");
	}
}
