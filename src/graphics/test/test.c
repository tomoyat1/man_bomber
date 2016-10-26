#include <stdio.h>
#include "man_bomber.h"
#include "mainDisplay.h"
#include <unistd.h>

int main(){
		struct metadata meta;
		meta.id=0;
		meta.wall_cnt=4;

		struct wall wa[3];
		wa[0].x=3; wa[0].y=0; wa[0].is_alive=1;
		wa[1].x=3; wa[1].y=2; wa[1].is_alive=1;
		wa[2].x=5; wa[2].y=6; wa[2].is_alive=0;
		wa[3].x=6; wa[3].y=6; wa[3].is_alive=1;

		struct player pl[4];
		pl[0].id=0;
		pl[0].x=0; pl[0].y=0;
		pl[0].is_alive=1;

		pl[1].id=1;
		pl[1].x=2; pl[1].y=6;
		pl[1].is_alive=1;

		pl[2].id=2;
		pl[2].x=6; pl[2].y=8;
		pl[2].is_alive=1;

		pl[3].id=3;
		pl[3].x=7; pl[3].y=3;
		pl[3].is_alive=0;

		struct bomb bo;
		init();
		while(1){
			refreshAll(&meta, &bo, pl, wa);
			if(end_flag==1) break;
			//usleep(15625000);
		}
		end();
		return 0;
}
