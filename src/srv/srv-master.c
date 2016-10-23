#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include "master.h"

void connect_to_slave(int i);

void connect_to_slave(int i)
{
	struct sockaddr_un fromaddr;
	int fromaddr_len;
	int slave_sock;

	/* Backlog is 4 */
	if (listen(domain_sock, 4) == -1) {
		perror("master domain socket listen");
	}
	fromaddr_len = sizeof(fromaddr);
	if ((slave_socks[i] = accept(domain_sock, (struct sockaddr *)&fromaddr, &fromaddr_len)) == -1)
		perror("Domain socket accept");
	/* foo */
}

/* TODO: Refactor */
int master_loop(char *addr_str, int port)
{
	int i;
	FILE *log, *err;
	time_t start_time;
	struct tm *tmptr;
	char fmt_time[64];

	struct sockaddr_in addr;
	struct sockaddr_in fromaddr;
	int addr_len;
	int fromaddr_len;
	int client_sock;
	int yes = 1;

	struct msghdr msg;
	struct cmsghdr *cmsg;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	struct iovec iov[1];

	log = fdopen(1, "a");
	err = fdopen(2, "a");
	/* Log server startup */
	start_time = time(NULL);
	tmptr = localtime(&start_time);
	strftime(fmt_time, sizeof(fmt_time), "%F %H:%M", tmptr);
	fprintf(stdout, "%s\n", fmt_time);
	fflush(stdout);

	if ((inet_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("inet socket");
	}
	setsockopt(inet_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
	addr.sin_family = AF_INET;
	if (!addr_str)
		addr.sin_addr.s_addr = INADDR_ANY;
	else
		addr.sin_addr.s_addr = inet_addr(addr_str);
	
	if (port < 0)
		addr.sin_port = htons(12345);
	else
		addr.sin_port = htons(port);
	addr_len = sizeof(addr);
	fromaddr_len = sizeof(fromaddr);
	if (bind(inet_sock, (struct sockaddr *)&addr, addr_len) == -1)
		perror("inet bind");
	/* Backlog is 4 */
	
	if (listen(inet_sock, 4) == -1) {
		perror("inet listen");
	}

	while (1) {
		if ((client_sock = accept(inet_sock, (struct sockaddr *)&fromaddr, &fromaddr_len)) == -1) {
			perror("inet accept");
		}
		/*
		 * Give fd to slave.
		 * auto-route to slave[0] for testing
		 * TODO: create function to query slave for availability.
		 */
		memset(&msg, 0, sizeof(msg));
		msg.msg_control = u.buf;
		msg.msg_controllen = sizeof(u.buf);
		iov[0].iov_base = &fromaddr;
		iov[0].iov_len = sizeof(fromaddr);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		*((int *)CMSG_DATA(cmsg)) = client_sock;
		if (sendmsg(slave_socks[0], &msg, 0) == -1) {
			perror("master socket send");
		}
		close(client_sock);
	}

	return 0;
}
