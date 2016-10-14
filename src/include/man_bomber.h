struct bomb_t {
	int id;
	int x;
	int y;
	int vx;
	int vy;
	int time; /* in game ticks */
	int aoe; /* area of effect */
};

struct player_t {
	int x;
	int y;
	int my_bombs;
};
