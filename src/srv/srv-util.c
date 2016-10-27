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
#include "man_bomber.h"
#include "man_bomber_config.h"
#include "master.h"
#include "srv.h"
#include "slave.h"
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
		if (recv_len = recv(fd, head, sizeof(int) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct bomb)) {
		if (recv_len = recv(fd, head, sizeof(struct bomb) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;
	if (total_len != sizeof(int) + sizeof(struct bomb))
		fprintf(stderr, "foo\n");

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
		if (recv_len = recv(fd, head, sizeof(int) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct player)) {
		if (recv_len = recv(fd, head, sizeof(struct player) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;
	if (total_len != sizeof(int) + sizeof(struct player))
		fprintf(stderr, "foo\n");

	return total_len;
}

int recv_single_wall(int fd, struct wall *buf, int *id)
{
	int recv_len;
	int len = 0;
	int total_len = 0;
	char *head;
	head = (char *)id;
	while (len < sizeof(int)) {
		if (recv_len = recv(fd, head, sizeof(int) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	head = (char *)buf;
	len = 0;
	while (len < sizeof(struct bomb)) {
		if (recv_len = recv(fd, head, sizeof(struct bomb) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;
	if (total_len != sizeof(int) + sizeof(struct player))
		fprintf(stderr, "foo\n");

	return total_len;
}

int send_end(int fd, int id)
{
	int end = END;
	if (send(fd, &end, sizeof(end), 0) == -1)
		return -1;
	if (send(fd, &id, sizeof(int), 0) == -1)
		return -1;
}

int send_single_bomb(int fd, struct bomb *b, int id)
{
	int bom = BOM;
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

int send_single_wall(int fd, struct wall *w, int id)
{
	/* hack */
	int wal = WAL;
	int len;
	int recv_len;
	int total_len;
	char *head;
	total_len = 0;
	head = (char *)&wal;

	len = 0;
	while (len < sizeof(int)) {
		if (recv_len = send(fd, head, sizeof(int) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	len = 0;
	head = (char *)&id;
	while (len < sizeof(int)) {
		if (recv_len = send(fd, head, sizeof(int) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;

	len = 0;
	head = (char *)w;
	while (len < sizeof(struct wall)) {
		if (recv_len = send(fd, head, sizeof(struct wall) - len, 0) == -1)
			return -1;
		len += recv_len;
		head += recv_len;
	}
	total_len += len;
	if (total_len != sizeof(int) + sizeof(int) + sizeof(struct wall))
		fprintf(stderr, "send foo\n");
	return 0;
}
