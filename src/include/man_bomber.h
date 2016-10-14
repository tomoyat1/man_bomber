struct bomb {
	int id;
	int x;
	int y;
	int vx; /* Velocity in X dir */
	int vy; /* Velocity in Y dir */
	int time; /* in game ticks */
	int aoe; /* area of effect */
}:

struct player {
	int x;
	int y;
	int bombs;
}

