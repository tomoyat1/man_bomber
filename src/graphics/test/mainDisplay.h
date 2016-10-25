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
void bomb_anime(int bbx, int bby);
int keyInput(char c);
void refreshAll(int ti);
void new_main(struct metadata *me, struct bomb *bo, struct player *pl, struct wall *wa);


#endif /* _MAINDISPLAY_H_ */
