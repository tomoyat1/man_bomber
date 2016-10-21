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

static int domain_sock;
static int inet_sock;
static int slaves[4];
static int slave_socks[4];

int master_loop(char *addr_str, int port);

#endif /* _MASTER_H_ */
