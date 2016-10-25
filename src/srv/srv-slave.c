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
#include "recv-data.h"

int init_slave(int i);
int slave_loop();
int send_end(int fd, long int id);
int send_single_bomb(int fd, struct bomb *b, int id);
int send_single_player(int fd, struct player *p, int id);
int send_state_to_master(int fd, int id, struct player *pl, struct bomb *bo, int bo_cnt);


int init_slave(int i)
{
	struct sockaddr_un addr;
	int addr_len;

	printf("slave %d initializing (pid: %d)\n", i, getpid());
	/* Connect to master */
	master_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/tmp/man-bomber-master.socket");
	addr_len = sizeof(addr);
	if (connect(master_sock, (struct sockaddr *)&addr, addr_len) == -1)
		perror("domain slave connect");
	slave_loop();
}

int send_end(int fd, long int id)
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

int send_state_to_master(int fd, int id, struct player *pl, struct bomb *bo, int bo_cnt)
{
	int i;
	/* Needs error handling */
	send_single_player(fd, pl, id);
	for (i = 0; i < bo_cnt; i++)
		send_single_bomb(fd, bo, id);
	send_end(fd, id);
}

int slave_loop()
{
	char buf[512];
	int recv_len;
	int msg_len = 0;
	int client_sock;

	struct sockaddr_in fromaddr;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	struct iovec iov[1];

	struct metadata cm;
	/* Hard-coded to max players */
	struct player pl[1];
	struct bomb *bo;
	struct wall *wa;

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
		if ((recvmsg(master_sock, &msg, MSG_WAITALL)) == -1)
			perror("slave domain recv");
		client_sock = *((int *)CMSG_DATA(CMSG_FIRSTHDR(&msg)));
#if ENABLE_HOGE_FUGA
		send(client_sock, "Hello\n", 6, 0);
		recv_len = recv(client_sock, buf, sizeof(buf), 0);
		if (strcmp(buf, "hoge\r\n") == 0)
			send(client_sock, "fuga\n", 5, 0);
		else
			send(client_sock, "???\n", 5, 0);
#else
		if ((recv_len = recv_meta(client_sock, &cm)) == -1)
			goto end_loop;
		else
			msg_len += recv_len;

		if ((recv_len = recv_player(client_sock,
		    pl,
		    1)) == -1)
			goto end_loop;
		else
			msg_len += recv_len;

		if (!(bo = (struct bomb *)malloc(sizeof(struct bomb)
		    * cm.bomb_cnt))) {
			fprintf(stderr, "(Slave: %d) out of memory\n", getpid());
			goto end_loop;
		}
		if ((recv_len = recv_bomb(client_sock,
		    bo,
		    cm.bomb_cnt)) == -1)
			goto free_bo;
		else
			msg_len += recv_len;

		/* Send state to master */
		send_state_to_master(master_sock, cm.id, pl, bo, cm.bomb_cnt);

		/* Wait for next tick i.e. wait until packet arrives on domain socket. */

		/* Send state to client and close */

#endif /* ENABLE_HOGE_FUGA */

free_bo:
		free(bo);
end_loop:
		close(client_sock);
	}
}
