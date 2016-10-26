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

int data_recv(int sd, struct metadata *data, struct player **pla, struct bomb **bo, struct wall **wa);


int data_recv(int sd, struct metadata *data, struct player **pla, struct bomb **bo, struct wall **wa) {
	int recv_len;
	int msg_len = 0;

	recv_len = recv_meta(sd, data);
	msg_len += recv_len;

	*pla = (struct player *)malloc(sizeof(struct player));
	recv_len = recv_player(sd, *pla, data->player_cnt);
	msg_len += recv_len;
	
	*bo = (struct bomb *)malloc(sizeof(struct bomb));
	recv_len = recv_bomb(sd, *bo, data->bomb_cnt);
	msg_len += recv_len;

	*wa = (struct wall *)malloc(sizeof(struct wall));
	recv_len = recv_wall(sd, *wa, data->wall_cnt);
	msg_len += recv_len;

	return 0;

}
