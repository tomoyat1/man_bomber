#ifndef _SLAVE__H_
#define _SLAVE__H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static int master_sock;

int init_slave();

#endif /* _SLAVE__H_ */
