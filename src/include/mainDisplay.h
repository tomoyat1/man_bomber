#ifndef _MAINDISPLAY_H_
#define _MAINDISPLAY_H_

#include "man_bomber.h"

static int end_flag=0;


void printWall();
void init();
int toSpos(int p, char axis);
int toGpos(int p, char axis);
void printFrame();
void printObj(int ax, int ay, char a);
void old_bomb_anime(int bbx, int bby);
void bomb_anime(int cnt, struct bomb *bo);
int keyInput(char c, struct player *pl , struct bomb *bo);
void refreshAll(int ti);
void new_main(struct metadata *me, struct bomb *bo, struct player *pl, struct wall *wa);
void end();


#endif /* _MAINDISPLAY_H_ */
