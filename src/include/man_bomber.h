#ifndef _MAN_BOMBER_H_
#define _MAN_BOMBER_H_

#include <stddef.h>

/* magic values with no particular meaning */
#define PLA 0x18fabe0d
#define BOM 0xdc3bae7b
#define WAL 0x582da3c1

/*
 * +----------------------------+
 * |Server-client message format|
 * +----------------------------+
 * Server-client messaging will be in the following format
 * -----------------------------------------------------------------------------
 *                     player_offset <--player_cnt------>
 * struct metadata PLA struct player ...... struct player
 *                     bomb_offset <--bomb_cnt-------->
 *                 BOM struct bomb   ...... struct bomb
 *                     wall_offset <--wall_cnt-------->
 *                 WAL struct wall   ...... struct wall
 * -----------------------------------------------------------------------------
 * where offsets and cnts are struct metadata fields.
 */

struct metadata {
	/* Client id */
	long int id;

	/*
	 * Tick count from beginning of round.
	 * srv->cli: contains server tick count. Clients should sync their
	 *           tick count with this value.
	 * cli->srv: contains client tick count. Server will gracefully
	 *           ignore all messages with a tick count which does not
	 *           match that of the server.
	 */
	int tick;

	/* Offset of player array from front of message in bytes. */
	size_t player_offset;

	/* Count of player structs. */
	size_t player_cnt;

	/* Offset of player array from front of message. */
	size_t bomb_offset;

	/* Count of bomb structs. */
	size_t bomb_cnt;

	/* Offset of player array from front of message. */
	size_t wall_offset;

	/* Count of wall structs. */
	size_t wall_cnt;
};

struct bomb {
	int id;
	int x;
	int y;
	int vx;
	int vy;

	/* in game ticks */
	int timer;

	/* area of effect */
	int aoe; 

	/* 0: dead, 1: alive */
	int is_alive; 
};

struct player {
	int x;
	int y;
	int bombs;
};

/* Destructable wall */
struct wall {
	int x;
	int y;

	/* 0: dead, 1: alive */
	int is_alive; 
};

int check_magic(int fd, int magic);

#endif /* _MAN_BOMBER_H_ */
