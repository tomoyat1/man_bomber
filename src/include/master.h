#ifndef _MASTER_H_
#define _MASTER_H_

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

#define FUSE 128
#define EXPLOSION_LEN 32

struct srv_state {
	long int tick;
};

static int domain_sock;
static int inet_sock;
static int slaves[4];
static int slave_socks[4];

static struct player players[4];
static struct list_node *bombs;
static struct list_node *walls;
static struct list_node *p_wait;
static struct list_node *b_wait;
static struct list_node *w_wait;
static struct srv_state state;

void connect_to_slave(int i);
int master_loop(char *addr_str, int port);

#endif /* _MASTER_H_ */
