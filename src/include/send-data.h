int send_bomb(int fd, struct bomb *data, int count);
int send_meta(int fd, struct metadata *data);
int send_player(int fd, struct player *data, int count);
int send_wall(int fd, struct wall *data, int count);