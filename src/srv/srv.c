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
#include "slave.h"

void spawn_slaves();
int srv_init();
void usage();

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
