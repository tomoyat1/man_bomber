struct bomb {
	int id;
	int x;
	int y;
	int vx;
	int vy;
	int time; /* in game ticks */
	int aoe; /* area of effect */
	int is_alive; /* 0: dead, 1: alive */
};

struct player {
	int x;
	int y;
};

/* Destructable wall */
struct wall {
	int x;
	int y;
	int is_alive; /* 0: dead, 1: alive */
};

struct client_data {
	unsigned int id;
	struct player p;
	/* Variable lenght array of struct bombs */
	struct *bomb bombs;
	/* Current tick. Ignore data if not in sync with server. */
	unsigned int cur_tick;
};

struct server_data {
	unsigned int id;
	struct player p;
	/* Variable length array of struct bombs */
	struct bomb *bombs;
	/* Variable length array of struct walls */
	struct wall *walls;
	/* Current tick. Ignore data if not in sync with server. */
	unsigned int cur_tick;
};
