#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "man_bomber.h"
#include "man_bomber_config.h"

#include "slave.h"

int init_slave(int i);
int slave_loop();

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

int slave_loop()
{
	char buf[512];
	int recv_len;
	int client_sock;

	struct sockaddr_in fromaddr;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	struct iovec iov[1];

	printf("Slave loop\n");
	while (1) {
		memset(buf, 0, sizeof(buf));
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
		//msg.msg_controllen = cmsg->cmsg_len;
		if ((recvmsg(master_sock, &msg, MSG_WAITALL)) == -1)
			perror("slave domain recv");
		client_sock = *((int *)CMSG_DATA(CMSG_FIRSTHDR(&msg)));
		send(client_sock, "Hello\n", 6, 0);
		recv_len = recv(client_sock, buf, sizeof(buf), 0);
#if ENABLE_HOGE_FUGA
		if (strcmp(buf, "hoge\r\n") == 0)
			send(client_sock, "fuga\n", 5, 0);
		else
			send(client_sock, "???\n", 5, 0);
#endif /* ENABLE_HOGE_FUGA */
		close(client_sock);
	}
}
