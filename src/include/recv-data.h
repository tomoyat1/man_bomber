int recv_bomb(int fd, struct bomb *data, int count);
int recv_meta(int fd, struct metadata *data);
int recv_player(int fd, struct player *data, int count);
int recv_wall(int fd, struct wall *data, int count);
