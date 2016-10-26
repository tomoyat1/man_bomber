#ifndef _SRV_H_
#define _SRV_H_

int recv_single_bomb(int fd, struct bomb *buf, int *id);
int recv_single_player(int fd, struct player *buf, int *id);
int recv_single_wall(int fd, struct wall *buf, int *id);
int recv_magic(int fd);
int send_end(int fd, int id);
int send_single_bomb(int fd, struct bomb *b, int id);
int send_single_player(int fd, struct player *p, int id);
int send_single_wall(int fd, struct wall *w, int id);
int send_state_to_master(int fd, int id, struct player *pl, struct bomb *bo, int bo_cnt);

#endif /* _SRV_H_ */
