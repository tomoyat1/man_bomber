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
#include "srv.h"

void bomb_clean(struct list_node **gone, struct list_node **hot);
void bomb_countdown(struct list_node **hot, struct list_node **burning, struct list_node **gone);
void bomb_kaboom(struct list_node **burning, struct list_node **hot);
void bomb_prime(struct list_node **queue, struct list_node **hot);
int bomb_equals_node_by_coords(struct bomb *b, struct list_node *n);
void clear_list(struct list_node **head);
void connect_to_slave(int i);
int count_players();
void handle_tick(int signal);
void init_game_server();
int is_blocked(struct player *pla, struct list_node *wal);
int is_coord_blocked(int x, int y, struct list_node *wal);
int is_crispy(struct player *pla, struct bomb *bom);
void kill_player(struct player *p);
unsigned int random_int();
int send_state_to_slave(int fd,
    int id,
    struct player *pl,
    struct bomb *bo,
    int bo_cnt,
    struct wall *wa,
    int wa_cnt);
struct bomb * search_bomb_by_coords(struct bomb *entry, struct list_node *head);
int slave_ready(int i);
void update_bombs();
void update_players();
int validate_bomb_coords(struct bomb *b);
int validate_player_coords(struct player *p);
int validate_destructable_wall_coords(int x, int y);
int validate_wall_coords(struct player *p);

void bomb_clean(struct list_node **gone, struct list_node **hot)
{
	struct list_node *cur, *tmp;
	struct bomb *b;
	struct bomb_ptr *ptr;

	/* Remove bombs one second after detonation */
	cur = *gone;
	while (cur) {
		tmp = cur->next;
		printf("cur = %p\n", cur);
		printf("cur->next = %p\n", cur->next);
		ptr = list_entry(struct bomb_ptr, cur, node);
		b = ptr->bptr;
		list_remove(&b->node, hot);
		printf("free: %p\n", b);
		free(b);
		list_remove(cur, gone);
		free(ptr);
		cur = tmp;
	}
}

void bomb_countdown(struct list_node **hot, struct list_node **burning, struct list_node **gone)
{
#define IS_BOOM(p) \
	(p->timer >= -EXPLOSION_LEN && p->timer <= 0)
#define IS_GONE(p) \
	p->timer < -(EXPLOSION_LEN + 32)

	struct list_node *cur;
	struct bomb *b;
	struct bomb_ptr *ptr;

	int i;

	/* Loop on game bombs list */
	cur = *hot;
	i = 0;
	while (cur) {
		(b = list_entry(struct bomb, cur, node))->timer--;
		i++;
		printf("ticking bomb %d: timer = %d\n", i, b->timer);
		printf("%d\n", b->timer);
		if (IS_BOOM(b)) {
			ptr = (struct bomb_ptr *)malloc(sizeof(struct bomb_ptr));
			ptr->bptr = b;
			/* add to burning */
			list_add(&ptr->node, burning);

		} else if (IS_GONE(b)) {
			ptr = (struct bomb_ptr *)malloc(sizeof(struct bomb_ptr));
			ptr->bptr = b;
			/* add to gone */
			list_add(&ptr->node, gone);
		}
		cur = cur->next;
	}
#undef IS_GONE
}

void bomb_kaboom(struct list_node **burning, struct list_node **hot)
{
	int i;
	struct list_node *cur, *tmp;;
	struct bomb *b;
	struct bomb_ptr *ptr;

	/* Hit detection for bombs upto 0.5s from detonation */
	cur = *burning;
	while (cur) {
		printf("kaboom\n");
		tmp = cur->next;
		ptr = list_entry(struct bomb_ptr, cur, node);
		b = ptr->bptr;
		/* Heavy-lifting of hit detection */
		for (i = 0; i < 4; i++) {
			if (is_crispy(&players[i], b))
				kill_player(&players[i]);
		}
		list_remove(cur, burning);
		free(ptr);
		cur = tmp;
	}
}

void bomb_prime(struct list_node **queue, struct list_node **hot)
{
	struct bomb *b;
	struct list_node *cur;
	/* Loop on update queue */
	cur = *queue;
	while (cur) {
		if (!search_bomb_by_coords(list_entry(struct bomb, cur, node),
		    *hot)
		    //&& validate_bomb_coords(list_entry(struct bomb, cur, node))) {
			) {
			if (b = (struct bomb *)malloc(sizeof(struct bomb))) {
				*b = *list_entry(struct bomb, cur, node);
				list_add(&(b->node), hot);
				b->timer = FUSE;
				b->aoe = 3;
			} else {
				fprintf(stderr, "(Slave %d) Out of memory\n", getpid());
			}
		}
		cur = cur->next;
	}
}

int bomb_equals_node_by_coords(struct bomb *b, struct list_node *n)
{
	int truth = 1;
	truth &= (list_entry(struct bomb, n, node)->x == b->x);
	truth &= (list_entry(struct bomb, n, node)->y == b->y);
	return truth;
}

/* 
 * This function is here because of memory release problems.
 * It does not belong in lib.
 */
void clear_list(struct list_node **head)
{
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

int count_players()
{
	int i;
	int cnt;
	cnt = 0;
	for (i = 0; i < 4; i++) {
		if (players[i].id >= 0)
			cnt++;
	}
	return cnt;
}

void handle_tick(int signal)
{
	int i;
	int bomb_cnt;
	int player_cnt;
	int wall_cnt;
	state.tick++;
	printf("TICK!: %ld\n", state.tick);
	update_players();
	update_bombs();

	player_cnt = count_players();
	bomb_cnt = list_count(&bombs);
	wall_cnt = list_count(&walls);

	for (i = 0; i < 4; i++) {
		if (state.slave_busy[i]) {
			send_state_to_slave(slave_socks[i],
			    state.slave_pla_id[i],
			    players,
			    list_entry(struct bomb, bombs, node),
			    bomb_cnt,
			    list_entry(struct wall, walls, node),
			    wall_cnt);
			state.slave_busy[i] = 0;
		}
	}

	clear_list(&b_wait);
	clear_list(&w_wait);
}

void init_game_server()
{
	int i;
	int dest_walls;
	int wal_x, wal_y;
	struct wall *new_wal;
	/* Open /dev/urandom */
	state.rng = open("/dev/urandom", O_RDONLY);
	/* Initialize game state lists */
	bombs = NULL;
	walls = NULL;
	memset(players, 0, sizeof(players));
	for (i = 0; i < 4; i++)
		players[i].is_alive = 1;
	/* For testing: will assign id to new clients upon connection in the future */
	for (i = 0; i < 4; i++)
		players[i].id = i;
	/* Assign starting positions statically for players 0 to 3 in this init phase */
	players[0].x = 0;
	players[0].y = 0;
	players[1].x = 14;
	players[1].y = 0;
	players[2].x = 0;
	players[2].y = 8;
	players[3].x = 14;
	players[3].y = 8;

	/* Initialize destructable walls */
	/*
	dest_walls = DEST_WALLS;
	while (dest_walls) {
		wal_x = random_int() % 15;
		wal_y = random_int() % 9;
		fprintf(stderr, "x: %d, y: %d\n", wal_x, wal_y);
		if (validate_destructable_wall_coords(wal_x, wal_y)
		    && !is_coord_blocked(wal_x, wal_y, walls)) {
			new_wal = (struct wall *)malloc(sizeof(struct wall));
			new_wal->x = wal_x;
			new_wal->y = wal_y;
			INIT_LIST_HEAD(&new_wal->node);
			list_add(&new_wal->node, &walls);
			fprintf(stderr, "inc walls\n");
			dest_walls--;
		}
	}
	*/

	/* Initialize wait lists */
	p_wait = NULL;
	b_wait = NULL;
	w_wait = NULL;

	/* Initialize tick count */
	state.tick = 0;
}

/* Destructable wall validation */
int is_blocked(struct player *pla, struct list_node *wal)
{
	int truth = 0;
	struct list_node *cur;
	struct bomb *curb;
	cur = wal;
	while (cur) {
		curb = list_entry(struct bomb, cur, node);
		truth |= (pla->x == curb->x && pla->y == curb->y);
		cur = cur->next;
	}
	return truth;
}

int is_coord_blocked(int x, int y, struct list_node *wal)
{
	int truth = 0;
	struct list_node *cur;
	struct bomb *curb;
	cur = wal;
	while (cur) {
		curb = list_entry(struct bomb, cur, node);
		truth |= (x == curb->x && y == curb->y);
		cur = cur->next;
	}
	return truth;
}

/* 爆弾のあたり判定 */
int is_crispy(struct player *pla, struct bomb *bom)
{
	int i;
	for (i = 1; i <= bom->aoe; i++){
		if( bom->x==pla->x && (bom->y+i==pla->y || bom->y-i==pla->y) ) return 1;
		if( bom->y==pla->y && (bom->x+i==pla->x || bom->x-i==pla->x) ) return 1;
	}
	return 0;
}

void kill_player(struct player *p)
{
	p->is_alive = 0;
}

unsigned int random_int()
{
	int rn;
	read(state.rng, &rn, sizeof(int));
	return rn;
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
	tick_spec.it_value.tv_sec = 0;
	tick_spec.it_value.tv_nsec = 15625000;;
	timer_settime(tick_tm, 0, &tick_spec, &tick_spec);

	memset(state.slave_busy, 0, sizeof(int) * 4);
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
			sigemptyset(&alrm);
			state.slave_busy[i] = 1;
			sigaddset(&alrm, SIGALRM);
			sigprocmask(SIG_BLOCK, &alrm, NULL);
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
			for (i = 0; i < 4 && !slave_ready(i); i++);
			printf("slave %d, fd = %d", i, slave_socks[i]);
			if (sendmsg(slave_socks[i], &msg, 0) == -1) {
				perror("master socket send");
			}
			close(client_sock);
			sigprocmask(SIG_UNBLOCK, &alrm, NULL);
		}
		for (i = 0; i < 4; i++) {
			int rlen;
			if (FD_ISSET(slave_socks[i], &rset)) {
				/* recv state from slave */
				/* queue state change */
				sigemptyset(&alrm);
				state.slave_busy[i] = 1;
				sigaddset(&alrm, SIGALRM);
				sigprocmask(SIG_BLOCK, &alrm, NULL);
				switch(recv_magic(slave_socks[i])) {
				case PLA:
					p = (struct player *)malloc(sizeof(struct player));
					if (rlen == -1)
						perror("recv_single_player");
					rlen = recv_single_player(slave_socks[i], p, &id);
					INIT_LIST_HEAD(&(p->node));
					list_add(&(p->node), &p_wait);
					state.slave_pla_id[i] = id;
					break;
				case BOM:
					b = (struct bomb *)malloc(sizeof(struct bomb));
					rlen = recv_single_bomb(slave_socks[i], b, &id);
					if (rlen == -1)
						perror("recv_single_bomb");
					INIT_LIST_HEAD(&b->node);
					if (!search_bomb_by_coords(b, b_wait))
						list_add(&(b->node), &b_wait);
					else
						fprintf(stderr,"dup bomb\n");
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

struct player * player_in_state(int id)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (players[i].id == id)
			return (players + i);
		fprintf(stderr, "FATAL ERROR: no player with id %x\n", id);
		return NULL;
	}
}

/* Search for entry in bomb list whose coordinates match entry */
struct bomb * search_bomb_by_coords(struct bomb *entry, struct list_node *head)
{
	struct list_node *cur = head;
	if (!head)
		return NULL;
	while (cur) {
		if (bomb_equals_node_by_coords(entry, cur))
			return list_entry(struct bomb, cur, node);
		cur = cur->next;
	}
	return NULL;
}

int send_state_to_slave(int fd,
    int id,
    struct player *pl,
    struct bomb *bo,
    int bo_cnt,
    struct wall *wa,
    int wa_cnt)
{
	int i;
	struct list_node *cur;
	struct metadata data;
	data.id = id;
	data.player_cnt = 4;
	data.bomb_cnt = bo_cnt;
	data.wall_cnt = wa_cnt;
	data.tick = state.tick;
	send(fd, &data, sizeof(struct metadata), 0);

	/* Needs error handling */
	for (i = 0; i < 4; i++)
		send_single_player(fd, &pl[i], id);
	cur = &bo->node;
	while (cur) {
		printf("Sending bomb\n");
		send_single_bomb(fd, list_entry(struct bomb, cur, node), id);
		cur = cur->next;
	}
	cur = &wa->node;
	while (cur) {
		send_single_wall(fd, list_entry(struct wall, cur, node), id);
		cur = cur->next;
	}
	send_end(fd, id);
}

int slave_ready(int i)
{
	return (!state.slave_busy[i]);
}

void update_bombs()
{
	struct bomb *foo[10];
	struct list_node *burning = NULL;
	struct list_node *gone = NULL;

	bomb_countdown(&bombs, &burning, &gone);
	/* fizz... */
	bomb_prime(&b_wait, &bombs);
	/* KABOOM BABY!! ...well actually, called for every tick a bomb is burning */
	bomb_kaboom(&burning, &bombs);
	bomb_clean(&gone, &bombs);
}

void update_players()
{
	int is_alive;
	int search_id;
	struct list_node *cur;
	struct player *p;
	struct player *queue_p;
	cur = p_wait;
	while (cur) {
		queue_p = list_entry(struct player, cur, node);
		if (!validate_player_coords(queue_p) || is_blocked(queue_p, walls)) {
			cur = cur->next;
			continue;
		}
		search_id = queue_p->id;
		if (p = player_in_state(search_id)) {
			is_alive = players[search_id].is_alive;
			*p = *list_entry(struct player, cur, node);
			p->is_alive = is_alive;
		}
		cur = cur->next;
	}
}

int validate_bomb_coords(struct bomb *b)
{
	int truth = 1;
	truth &= (b->x >= 0 && b->x < 15);
	truth &= (b->y >= 0 && b->y < 9);
	truth &= !(b->x % 2 && b->y % 2);
	if (!truth)
		fprintf(stderr, "Invalid bomb coords\n");
	return truth;
}

int validate_destructable_wall_coords(int x, int y)
{
	int truth = 1;
	truth &= (x >= 0 && x < 15);
	truth &= (y >= 0 && y < 9);
	truth &= !(x % 2 && y % 2);
	truth &= !(x == 0 && (y == 0 || y == 1) && x == 1 && y == 0);
	truth &= !(x == 14 && (y == 0 || y == 1) && x == 13 && y == 0);
	truth &= !(x == 14 && (y == 8 || y == 7) && x == 13 && y == 8);
	truth &= !(x == 0 && (y == 8 || y == 7) && x == 1 && y == 8);
	if (!truth)
		fprintf(stderr, "Invalid coords\n");
	return truth;
}

int validate_player_coords(struct player *p)
{
	int truth = 1;
	truth &= (p->x >= 0 && p->x < 15);
	truth &= (p->y >= 0 && p->y < 9);
	truth &= !(p->x % 2 && p->y % 2);
	if (!truth)
		fprintf(stderr, "Invalid player coords\n");
	return truth;
}

int validate_wall_coords(struct player *p)
{
	int truth = 1;
	truth &= (p->x >= 0 && p->x < 15);
	truth &= (p->y >= 0 && p->y < 9);
	truth &= !(p->x % 2 && p->y % 2);
	if (!truth)
		fprintf(stderr, "Invalid wall coords\n");
	return truth;
}
