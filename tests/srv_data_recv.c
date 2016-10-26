#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "man_bomber.h"
#include "man_bomber_config.h"

int main(int argc, char **argv)
{
	int i;
	int sock;
	int pla = PLA;
	int bom = BOM;
	int wal = WAL;
	struct sockaddr_in addr;
	struct metadata data;
	struct player players[1];
	struct bomb bombs[10];
	sock = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	memset(&data, 0, sizeof(data));
	data.id = 0xDEADBEEF;
	data.player_cnt = 1;
	data.player_offset = sizeof(struct metadata);
	data.bomb_cnt = 10;
	data.bomb_offset = sizeof(struct metadata)
	    + sizeof(struct player) * data.player_cnt;
	data.wall_cnt = 0;
	data.wall_offset = 0;

	players[0].id = 0;

	for (i = 0; i < 10; i++) {
		bombs[i].x = i;
		bombs[i].y = 2 * i;
	}
	printf("Sending payload\n");
	send(sock, &data, sizeof(data), 0);
	send(sock, &pla, sizeof(int), 0);
	send(sock, players, sizeof(players), 0);
	send(sock, &bom, sizeof(int), 0);
	send(sock, bombs, sizeof(bombs), 0);
	return 0;
}
