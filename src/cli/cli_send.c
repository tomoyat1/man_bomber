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

int data_send(int sd, struct metadata *data, struct player **pla, struct bomb **bo, struct wall **wa);


int data_send(int sd, struct metadata *data, struct player **pla, struct bomb **bo, struct wall **wa) {
        int send_len;
        int msg_len = 0;

        send_len = send_meta(sd, data);
        msg_len += send_len;

        *pla = (struct player *)malloc(sizeof(struct player));
        send_len = send_player(sd, *pla, 1);
	 msg_len += send_len;
	free(*pla);

        *bo = (struct bomb *)malloc(sizeof(struct bomb));
        send_len = send_bomb(sd, *bo, data->bomb_cnt);
        msg_len += send_len;
	free(*bo);

        *wa = (struct wall *)malloc(sizeof(struct wall));
        send_len = send_wall(sd, *wa, data->wall_cnt);
        msg_len += send_len;
	free(*wa);	

        return 0;

}

