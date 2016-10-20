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

#include "slave.h"

int slave_loop()
{
	char buf[512];
	int read_len;
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;

	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	msg.msg_controllen = cmsg->cmsg_len;

	printf("Slave loop\n");
	while (1) {
		if (recvmsg(master_sock, &msg, 0) == -1)
			perror("slave domain recv");
		printf("fd: %d\n", *((int *)CMSG_DATA(CMSG_FIRSTHDR(&msg))));
		while (1) {
			printf("socket recved\n");
			sleep(3);
		}
	}
}

int init_slave(int i)
{
	struct sockaddr_un addr;
	int addr_len;

	printf("slave %d initializing (pid: %d)\n", i, getpid());
	/* Connect to master */
	master_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/var/tmp/man-bomber-master.socket");
	addr_len = sizeof(addr);
	if (connect(master_sock, (struct sockaddr *)&addr, addr_len) == -1)
		perror("domain slave connect");

	slave_loop();
}
