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
#include "send-data.h"

#define CNT_TO_SIZE(type, cnt) \
	(sizeof(type) * cnt)

int send_meta(int fd, struct metadata *data)
{
	size_t send_len, len;
	char errmsg[64];
	send_len = send(fd, data, sizeof(struct metadata), 0);
		if (send_len < 0) {
			snprintf(errmsg, 64, "(Slave: %d)Metadata send error",getpid());
			perror("metadata");
			len = send_len;
			goto exit;
		}
exit:
	return send_len;
}

/*
* int send_bomb(int fd, struct player *data, size_t size);
* int fd: File descriptor for socket to send to
* struct bomb *data: Buffer to write received data.
* int count: Number of struct bomb to receive.
*
* Returns bytes sent. Returns -1 on error.
*/
int send_bomb(int fd, struct bomb *data, int count)
{
	int bom;
	size_t send_len, len;
	char errmsg[64];

	bom = BOM;
	len = 0;
	if (!check_magic(fd, BOM))
		return -1;
	send_len = send(fd, &bom, sizeof(int), 0);
	len += send_len;
	send_len = send(fd, data, CNT_TO_SIZE(struct bomb, count), 0);
		if (send_len < 0) {
			snprintf(errmsg, 64, "(Slave: %d) Bomb send error", getpid());
			perror("metadata");
			len = send_len;
			goto exit;
		}
	len += send_len;
exit:
	return len;
}

/*
* int send_player(int fd, struct player *data, size_t size);
* int fd: File descriptor for socket to send to
* struct player *data: Buffer to write received data.
* int count: Number of struct player to receive.
*
* Returns bytes sent. Returns -1 on error.
*/
int send_player(int fd, struct player *data, int count)
{
	int pla;
	size_t send_len, len;
	char errmsg[64];

	pla = PLA;
	len = 0;
	if (!check_magic(fd, PLA))
		return -1;
	send_len = send(fd, &pla, sizeof(int), 0);
	len += send_len;
	send_len = send(fd, data, CNT_TO_SIZE(struct player, count) - len, 0);
		if (send_len < 0) {
			snprintf(errmsg,64,"(Slave: %d) Player send error",getpid());
			perror("metadata");
			len = send_len;
			goto exit;
		}
	len += send_len;
exit:
	return len;
}

/*
* int send_wall(int fd, struct wall *data, size_t size);
* int fd: File descriptor for socket to send to
* struct wall *data: Buffer to write received data.
* int count: Number of struct wall to receive.
*
* Returns bytes sent. Returns -1 on error.
*/
int send_wall(int fd, struct wall *data, int count)
{
	int wal;
	size_t send_len, len;
	char errmsg[64];

	len = 0;
	wal = WAL;
	send_len = send(fd, &wal, sizeof(int), 0);
	len += send_len;
	send_len = send(fd, data, CNT_TO_SIZE(struct wall, count) - len, 0);
		if (send_len < 0) {
			snprintf(errmsg,64,"(Slave: %d) Wall send error", getpid());
			perror("wall");
			len = send_len;
			goto exit;
		}
	len += send_len;
exit:
	return len;
}
