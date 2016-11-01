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

#include "list.h"
#include "recv-data.h"
#include "send-data.h"
#include "slave.h"
#include "srv.h"

int init_slave(int i);
int recv_state_from_master(
    int fd,
    struct metadata *data,
    struct player p[4],
    struct bomb **b,
    struct wall **w);
int send_state_to_client(
    int fd,
    struct metadata *data,
    struct player *pl,
    struct bomb *bo,
    int bo_cnt,
    struct wall *wa,
    int wa_cnt);
int slave_loop();

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

/* Give pointer to pointer that will point to buf, which is allocated by this function. */
int recv_state_from_master(int fd,
    struct metadata *data,
    struct player *p,
    struct bomb **b,
    struct wall **w)
{
	int i;
	int rlen;
	int len;
	int id;
	struct player *pptr;
	struct bomb *cur_b;
	struct wall *cur_w;

	char buf[1024];

	recv_meta(fd, data);

	if (data->id < 0 || data->id >= 4) {
		fprintf(stderr, "invalid id");
		//abort();
	}

	*b = (struct bomb *)malloc(sizeof(struct bomb) * data->bomb_cnt);
	*w = (struct wall *)malloc(sizeof(struct wall) * data->wall_cnt);
	cur_b = *b;
	cur_w = *w;
	len = data->player_cnt * (sizeof(struct player) + sizeof(int))
	    + data->bomb_cnt * (sizeof(struct bomb) + sizeof(int))
	    + data->wall_cnt * (sizeof(struct wall) + sizeof(int))
	    + sizeof(int) /* end id */
	    + sizeof(int) * (1 + data->player_cnt + data->bomb_cnt + data->wall_cnt);
	fprintf(stderr, "player_cnt = %d\n", (int)data->player_cnt);
	fprintf(stderr, "bomb_cnt = %d\n", (int)data->bomb_cnt);
	fprintf(stderr, "wall_cnt = %x\n", (int)data->wall_cnt);
	fprintf(stderr, "len = %d\n", len);

	for (i = 0; i < 4; i++)
		p[i].id = -1;
	while (len > 0) {
		switch(recv_magic(master_sock)) {
			case PLA:
				pptr = (struct player *)malloc(sizeof(struct player));
				rlen = recv_single_player(master_sock, pptr, &id);
				if (rlen == -1)
					perror("recv_single_player");
				if (rlen != sizeof(struct player) + sizeof(int))
					fprintf(stderr, "struct player size not match\n");
				if (pptr-> id < 0 || pptr->id > 3) {
					fprintf(stderr, "(Slave %d) FATAL ERROR: INVALID PLAYER ID\n", getpid());
					return -1;
				}
				p[pptr->id] = *pptr;
				free(pptr);
				len -= rlen;
				break;
			case BOM:
				rlen = recv_single_bomb(master_sock, cur_b, &id);
				if (rlen == -1)
					perror("recv_single_bomb");
				if (rlen != sizeof(struct bomb) + sizeof(int))
					fprintf(stderr, "struct bomb size not match\n");
				cur_b++;
				len -= rlen;
				break;
			case WAL:
				rlen = recv_single_wall(master_sock, cur_w, &id);
				if (rlen == -1)
					perror("recv_single_wall");
				if (rlen != sizeof(struct wall) + sizeof(int))
					fprintf(stderr, "struct wall size not match\n");
				cur_w++;
				len -= rlen;
				break;
			case END:
				rlen = recv(master_sock, &id, sizeof(int), 0);
				if (rlen != sizeof(int))
					fprintf(stderr, "end size not match\n");
				len -= rlen;
				break;
			default:
				fprintf(stderr, "not magic\n");
				return -1;
		}
		/* Magic bytes */
		len -= 4;
	}
}

int send_state_to_client(
    int fd,
    struct metadata *data,
    struct player *pl,
    struct bomb *bo,
    int bo_cnt,
    struct wall *wa,
    int wa_cnt)
{
	if (send_meta(fd, data) == -1)
		perror("send_meta");
	if (send_player(fd, pl, 4) == -1)
		perror("send_meta");
	fprintf(stderr, "bo_cnt = %d\n", bo_cnt);
	if (send_bomb(fd, bo, bo_cnt) == -1)
		perror("send_meta");
	fprintf(stderr, "wa_cnt = %d\n", wa_cnt);
	if (send_wall(fd, wa, wa_cnt) == -1)
		perror("send_meta");
}

int send_state_to_master(int fd, int id, struct player *pl, struct bomb *bo, int bo_cnt)
{
	int i;
	/* Needs error handling */
	send_single_player(fd, pl, id);
	for (i = 0; i < bo_cnt; i++)
		send_single_bomb(fd, &bo[i], id);
	send_end(fd, id);
}

int slave_loop()
{
	char buf[512];
	int recv_len; int msg_len = 0;
	int client_sock;

	struct sockaddr_in fromaddr;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	struct iovec iov[1];

	struct metadata mc;
	struct metadata ms;
	/* Hard-coded to max players */
	struct player pc[1];
	struct player ps[4];
	struct bomb *bc;
	struct bomb *bs;
	struct wall *wc;
	struct wall *ws;

	printf("Slave loop\n");
	while (1) {
		memset(buf, 0, sizeof(buf));
		memset(&msg, 0, sizeof(msg)); msg.msg_control = u.buf;
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
		fprintf(stderr, "slave pid %d\n", getpid());
#if ENABLE_HOGE_FUGA
		send(client_sock, "Hello\n", 6, 0);
		recv_len = recv(client_sock, buf, sizeof(buf), 0);
		if (strcmp(buf, "hoge\r\n") == 0)
			send(client_sock, "fuga\n", 5, 0);
		else
			send(client_sock, "???\n", 5, 0);
#else
		if ((recv_len = recv_meta(client_sock, &mc)) == -1)
			goto end_loop;
		else
			msg_len += recv_len;

		if ((recv_len = recv_player(client_sock,
		    pc,
		    1)) == -1)
			goto end_loop;
		else
			msg_len += recv_len;

		if (!(bc = (struct bomb *)malloc(sizeof(struct bomb)
		    * mc.bomb_cnt))) {
			fprintf(stderr, "(Slave: %d) out of memory\n", getpid());
			goto end_loop;
		}
		if ((recv_len = recv_bomb(client_sock,
		    bc,
		    mc.bomb_cnt)) == -1)
			goto free_bc;
		else
			msg_len += recv_len;

		/* Send state to master */
		send_state_to_master(master_sock, mc.id, pc, bc, mc.bomb_cnt);
		/* Wait for next tick i.e. wait until packet arrives on domain socket. */
		if (recv_state_from_master(master_sock, &ms, ps, &bs, &ws) == -1)
			fprintf(stderr, "(Slave %d) recv_state_from_master failed\n", getpid());
		else
			fprintf(stderr, "recved state from master\n");
		/* Send state to client and close */
		send_state_to_client(client_sock, &ms, ps, bs, ms.bomb_cnt, ws, ms.wall_cnt);
		fprintf(stderr, "sent state to client\n");

#endif /* ENABLE_HOGE_FUGA */

free_bc:
		free(bc);
end_loop:
		close(client_sock);
	}
}
