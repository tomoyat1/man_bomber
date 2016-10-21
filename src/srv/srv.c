#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "man_bomber.h"
#include "man_bomber_config.h"
#include "master.h"
#include "slave.h"

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
	strcpy(addr.sun_path, "/var/tmp/man-bomber-master.socket");
	unlink("/var/tmp/man-bomber-master.socket");
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
	pidfile = fopen("/var/tmp/manbomber-srv.pid", "w+");
	fprintf(pidfile, "%d\n", getpid());
	fflush(pidfile);
	fclose(pidfile);

	/* fd handling */
	logfd = open("/var/tmp/manbomber-srv.log",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	errfd = open("/var/tmp/manbomber-srv.error",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	close(2);
	dup(errfd);
	close(1);
	dup(logfd);
	close(0);

	/* Span slaves */
	spawn_slaves();
}

int main(int argc, char **argv)
{
	int srv_loop_ret;
	pid_t pid, sid;
	printf("Start $ man bomber server version %d.%d\n",
	    man_bomber_VERSION_MAJOR,
	    man_bomber_VERSION_MINOR);
	pid = fork();
	if (pid == 0) {
		/* Start server loop */
		umask(0);
		srv_init();
		sid = setsid();
		srv_loop_ret = master_loop();
		exit(srv_loop_ret);
	} else {
		printf("Server started as pid: %d\n", pid);
		exit(0);
	}
}
