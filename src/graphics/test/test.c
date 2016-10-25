#include <stdio.h>
#include "man_bomber.h"
#include "mainDisplay.h"

int main(){
		struct metadata meta;
		meta.id=0;
		meta.wall_cnt=3;

		struct wall wa[3];
		wa[0].x=3; wa[0].y=0;
		wa[1].x=3; wa[1].y=2;
		wa[2].x=5; wa[2].y=6;

		struct player pl[4];
		pl[0].id=0;
		pl[0].x=0; pl[0].y=0;
		pl[0].is_alive=1;

		pl[1].id=0;
		pl[1].x=2; pl[1].y=6;
		pl[1].is_alive=1;

		pl[2].id=0;
		pl[2].x=6; pl[2].y=8;
		pl[2].is_alive=1;

		pl[3].id=0;
		pl[3].x=7; pl[3].y=3;
		pl[3].is_alive=0;

		struct bomb bo;

		new_main(&meta, &bo, pl, wa);

		return 0;
}
