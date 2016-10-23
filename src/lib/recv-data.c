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
#include "recv-data.h"

#define CNT_TO_SIZE(type, cnt) \
	(sizeof(type) * cnt)

int recv_meta(int fd, struct metadata *data)
{
	size_t recv_len, len;
	char buf[sizeof(struct metadata)];
	char *head = buf;
	char errmsg[64];
	len = 0;
	while (len < sizeof(struct metadata)) {
		recv_len = recv(fd, head, (sizeof(buf) - len), 0);
		if (recv_len < 0) {
			snprintf(errmsg,
			    64,
			    "(Slave: %d) Metadata recv error",
			    getpid());
			perror("metadata");
			goto exit;
		}
		len += recv_len;
		head += recv_len;
	}
	memcpy(data, buf, sizeof(struct metadata));
exit:
	return len;
}

/*
 * int recv_bomb(int fd, struct player *data, size_t size);
 * int fd: File descriptor for socket to recv from
 * struct bomb *data: Buffer to write received data.
 * int count: Number of struct bomb to receive.
 *
 * Returns bytes received. Returns -1 on error.
 */
int recv_bomb(int fd, struct bomb *data, int count)
{
	size_t recv_len, len;
	char buf[512];
	char *head = (char *)data;
	char errmsg[64];
	len = 0;
	if (!check_magic(fd, BOM))
		return -1;
	while (len < CNT_TO_SIZE(struct bomb, count)) {
		recv_len = recv(fd, buf, CNT_TO_SIZE(struct bomb, count) - len, 0);
		if (recv_len < 0) {
			snprintf(errmsg, 64, "(Slave: %d) Bomb recv error", getpid());
			perror("metadata");
			len = recv_len;
			goto exit;
		}
		memcpy(head, buf, recv_len);
		len += recv_len;
		head += recv_len;
	}
exit:
	return len;
}
 
/*
 * int recv_player(int fd, struct player *data, size_t size);
 * int fd: File descriptor for socket to recv from
 * struct player *data: Buffer to write received data.
 * int count: Number of struct player to receive.
 *
 * Returns bytes received. Returns -1 on error.
 */
int recv_player(int fd, struct player *data, int count)
{
	size_t recv_len, len;
	char buf[512];
	char *head = (char *)data;
	char errmsg[64];
	len = 0;
	if (!check_magic(fd, PLA))
		return -1;
	while (len < CNT_TO_SIZE(struct player, count)) {
		recv_len = recv(fd,
		    buf,
		    CNT_TO_SIZE(struct player, count) - len,
		    0);
		if (recv_len < 0) {
			snprintf(errmsg,
			    64,
			    "(Slave: %d) Player recv error",
			    getpid());
			perror("metadata");
			len = recv_len;
			goto exit;
		}
		memcpy(head, buf, recv_len);
		len += recv_len;
		head += recv_len;
	}
exit:
	return len;
}

/*
 * int recv_wall(int fd, struct wall *data, size_t size);
 * int fd: File descriptor for socket to recv from
 * struct wall *data: Buffer to write received data.
 * int count: Number of struct wall to receive.
 *
 * Returns bytes received. Returns -1 on error.
 */
int recv_wall(int fd, struct wall *data, int count)
{
	size_t recv_len, len;
	char buf[512];
	char *head = (char *)data;
	char errmsg[64];
	len = 0;
	if (!check_magic(fd, WAL))
		return -1;
	while (len < CNT_TO_SIZE(struct wall, count)) {
		recv_len = recv(fd,
		    buf,
		    CNT_TO_SIZE(struct wall, count) - len,
		    0);
		if (recv_len < 0) {
			snprintf(errmsg,
			    64,
			    "(Slave: %d) Wall recv error",
			    getpid());
			perror("wall");
			len = recv_len;
			goto exit;
		}
		memcpy(head, buf, recv_len);
		len += recv_len;
		head += recv_len;
	}
exit:
	return len;
}
