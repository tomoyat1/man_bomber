#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "man_bomber.h"
#include "master.h"

int bomb_equals_node_by_coords(struct bomb *b, struct list_node *n);
void clear_list(struct list_node **head);
void connect_to_slave(int i);
void handle_tick(int signal);
void init_game_server();
int recv_magic(int fd);
int recv_single_bomb(int fd, struct bomb *buf, int *id);
int recv_single_player(int fd, struct player *buf, int *id);
struct bomb * search_bomb_by_coords(struct bomb *entry, struct list_node *head);

int bomb_equals_node_by_coords(struct bomb *b, struct list_node *n)
{
	int truth = 0;
	truth &= (list_entry(struct bomb, n, node)->x == b->x);
	return truth;
}

/* 
 * This function is here because of memory release problems.
 * It does not belong in lib.
 */
void clear_list(struct list_node **head)
{
	/* This function is FUCKED: Double freed */
	void *payload;
	while (*head) {
		payload = list_entry(struct generic, *head, node);
		list_remove(*head, head);
		free(payload);
	}
}

void connect_to_slave(int i) {
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

void handle_tick(int signal)
{
	state.tick++;
	printf("TICK!: %ld\n", state.tick);

	/* Nuke wait queue */
	clear_list(&p_wait);
	clear_list(&b_wait);
	clear_list(&w_wait);
}

void init_game_server()
{
	/* Initialize game state lists */
	bombs = NULL;
	walls = NULL;

	/* Initialize wait lists */
	p_wait = NULL;
	b_wait = NULL;
	w_wait = NULL;

	/* Initialize tick count */
	state.tick = 0;
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

	fd_set rset;
	struct timeval to;
	int select_res;

	struct player *p;
	struct bomb *b;
	struct wall *w;
	int id;

	struct sigevent tick_ev;
	timer_t tick_tm;
	struct itimerspec tick_spec;
	sigset_t alrm;

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

	/* Initialize list heads */
	p = NULL;
	b = NULL;
	w = NULL;

	/* Initialize server state */
	init_game_server();

	/* Register SIGALRM */
	signal(SIGALRM, handle_tick);

	/* tick timer */
	tick_ev.sigev_notify = SIGEV_SIGNAL;
	tick_ev.sigev_signo = SIGALRM;
	timer_create(CLOCK_REALTIME, &tick_ev, &tick_tm);
	tick_spec.it_interval.tv_sec = 0;
	tick_spec.it_interval.tv_nsec = 15625000;
	tick_spec.it_value.tv_sec = 1;
	tick_spec.it_value.tv_nsec = 0;
	timer_settime(tick_tm, 0, &tick_spec, &tick_spec);

	while (1) {
		do {
			FD_ZERO(&rset);
			for (i = 0; i < 4; i++)
				FD_SET(slave_socks[i], &rset);
			/* inet_sock is the last opened fd */
			FD_SET(inet_sock, &rset);
			to.tv_sec = 0;
			to.tv_usec = 15625;
			/* FreeBSDのkqueue(2)使いたい... */
			select_res = select(inet_sock + 1, &rset, NULL, NULL, &to);
		} while (select_res <= 0);

		if (FD_ISSET(inet_sock, &rset)) {
			/*
			 * accept(2) connection in master.
			 * Following accept(2) call won't block.
			 */
			if ((client_sock = accept(inet_sock, (struct sockaddr *)&fromaddr, &fromaddr_len)) == -1) {
				perror("inet accept");
			}
			/*
			 * Give client_sock to slave.
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
		for (i = 0; i < 4; i++) {
			int rlen;
			if (FD_ISSET(slave_socks[i], &rset)) {
				/* recv state from slave */
				/* queue state change */
				sigemptyset(&alrm);
				sigaddset(&alrm, SIGALRM);
				sigprocmask(SIG_BLOCK, &alrm, NULL);
				switch(recv_magic(slave_socks[i])) {
				case PLA:
					p = (struct player *)malloc(sizeof(struct player));
					//INIT_LIST_HEAD(&p->node);
					if (rlen == -1)
						perror("recv_single_player");
					rlen = recv_single_player(slave_socks[i], p, &id);
					p->id = 0xdeadbeef;
					list_add(&(p->node), &p_wait);
					break;
				case BOM:
					b = (struct bomb *)malloc(sizeof(struct bomb));
					//INIT_LIST_HEAD(&b->node);
					rlen = recv_single_bomb(slave_socks[i], b, &id);
					if (rlen == -1)
						perror("recv_single_bomb");
					if (!search_bomb_by_coords(b, b_wait))
						list_add(&(b->node), &b_wait);
					break;
				case END:
					recv(slave_socks[i], &id, sizeof(int), 0);
					break;
				default:
					fprintf(stderr, "not magic\n");
				}
				sigprocmask(SIG_UNBLOCK, &alrm, NULL);
				/* fall through */
			}
		}
	} return 0;
}

/* Search for entry in bomb list whose coordinates match entry */
struct bomb * search_bomb_by_coords(struct bomb *entry, struct list_node *head)
{
	struct list_node *cur = head;
	if (!head)
		return NULL;
	while (bomb_equals_node_by_coords(entry, cur) && cur->next != NULL)
		cur = cur->next;
	if (!cur->next)
		return NULL;
	else
		return list_entry(struct bomb, cur, node);
}

struct player * search_player(int id)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (players[i].id == id)
			return (players + i);
		fprintf(stderr, "FATAL ERROR: no player with id %d\n", id);
		return NULL;
	}
}

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
