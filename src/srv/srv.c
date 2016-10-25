#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "list.h"
#include "man_bomber.h"
#include "man_bomber_config.h"
#include "master.h"
#include "srv.h"
#include "slave.h"

void spawn_slaves();
int srv_init();
void usage();

int recv_magic(int fd)
{
	int magic;
	int rlen;
	rlen = recv(fd, &magic, sizeof(int), 0);
	switch (magic) {
		case PLA:
			/* Fall-through */
		case BOM:
			/* Fall-through */
		case WAL:
			/* Fall-through */
		case END:
			return magic;
			break;
		default:
			fprintf(stderr, "(Master) Not magic: %x\n", magic);
			return -1;
	}
}

int main(int argc, char **argv)
{
	int srv_loop_ret;
	char c;
	char *addr = NULL;
	int port = -1;
	pid_t pid, sid;
	printf("Start $ man bomber server version %d.%d\n",
	    man_bomber_VERSION_MAJOR,
	    man_bomber_VERSION_MINOR);
	/* Parse options */
	while ((c = getopt(argc, argv, "b:p:")) != -1) {
		switch (c)  {
		case 'b':
			addr = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case '?':
			usage();
			return 1;
		default:
			abort();
		}
	}
	pid = fork();
	if (pid == 0) {
		/* Start server loop */
		umask(0);
		srv_init();
		sid = setsid();
		srv_loop_ret = master_loop(addr, port);
		exit(srv_loop_ret);
	} else {
		printf("Server started as pid: %d\n", pid);
		exit(0);
	}
}

int recv_single_bomb(int fd, struct bomb *buf, int *id)
{
	int recv_len;
	int len = 0;
	int total_len = 0;
	char *head;
	head = (char *)id;
	while (len < sizeof(int)) {
		if ((recv_len = recv(fd, head, sizeof(int) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct bomb)) {
		if ((recv_len = recv(fd, head, sizeof(struct bomb) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	return total_len;
}

int recv_single_player(int fd, struct player *buf, int *id)
{
	int recv_len;
	int len = 0;
	int total_len = 0;
	char *head;
	head = (char *)id;
	while (len < sizeof(int)) {
		if ((recv_len = recv(fd, head, sizeof(int) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct player)) {
		if ((recv_len = recv(fd, head, sizeof(struct player) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	return total_len;
}

int recv_single_wall(int fd, struct wall *buf, int *id)
{
	int recv_len;
	int len = 0;
	int total_len = 0;
	char *head;
	head = (char *)id;
	while (len < sizeof(int)) {
		if ((recv_len = recv(fd, head, sizeof(int) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct bomb)) {
		if ((recv_len = recv(fd, head, sizeof(struct bomb) - len, 0)) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	return total_len;
}

int send_end(int fd, int id)
{
	int end = END;
	if (send(fd, &end, sizeof(end), 0) == -1)
		return -1;
	if (send(fd, &id, sizeof(int), 0) == -1)
		return -1;
}

int send_single_bomb(int fd, struct bomb *b, int id)
{
	int  bom = BOM;
	if (send(fd, &bom, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, &id, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, b, sizeof(struct bomb), 0) == -1)
		return -1;
}

int send_single_player(int fd, struct player *p, int id)
{
	int pla = PLA;
	if (send(fd, &pla, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, &id, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, p, sizeof(struct player), 0) == -1)
		return -1;
	return 0;
}

int send_single_wall(int fd, struct wall *w, int id)
{
	int  bom = BOM;
	if (send(fd, &bom, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, &id, sizeof(int), 0) == -1)
		return -1;
	if (send(fd, w, sizeof(struct bomb), 0) == -1)
		return -1;
}

void spawn_slaves()
{
	int i;
	pid_t tmp;
	struct sockaddr_un addr;
	int addr_len;

	/* Open domain socket and bind */
	if ((domain_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("domain socket socket\n");
	}
	addr.sun_family = AF_UNIX;
	addr_len = sizeof(addr);
	strcpy(addr.sun_path, "/tmp/man-bomber-master.socket");
	unlink("/tmp/man-bomber-master.socket");
	if (bind(domain_sock, (struct sockaddr *)&addr, addr_len) == -1) {
		perror("domain socket bind\n");
		exit(1);
	}

	for (i = 0; i < 4; i++) {
		tmp = fork();
		/* Domain socket foo */
		if (tmp != 0) {
			slaves[i] = tmp;
			connect_to_slave(i);
		} else {
			/* Will not return till server death */
			init_slave(i);
			break;
		}
	}
}

int srv_init()
{
	FILE *pidfile;
	int logfd, errfd;
	/* pid file */
	pidfile = fopen("/tmp/manbomber-srv.pid", "w+");
	fprintf(pidfile, "%d\n", getpid());
	fflush(pidfile);
	fclose(pidfile);

	/* fd handling */
	logfd = open("/tmp/manbomber-srv.log",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	errfd = open("/tmp/manbomber-srv.error",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	close(2);
	dup(errfd);
	close(1);
	dup(logfd);
	close(0);

	/* Span slaves */
	spawn_slaves();
}

void usage()
{
	fprintf(stderr, "manbomber-srv [-b address] [-p port]\n");
	fprintf(stderr, "  -b <value>\tIPv4 address to bind to. Defaults to 0.0.0.0\n");
	fprintf(stderr, "  -p <value>\tPort number bind to. Defaults to 12345\n");
}
