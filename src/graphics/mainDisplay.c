#include <curses.h>
#include <unistd.h>
#define offsetX 20
#define offsetY 30
#define FPS 50000  // sleep time in micro sec
//#include "man_bomber.h"


int end_flag=0;
int x=10,y=10;
int width=38, height=15;


void init(){
		initscr();
		cbreak();
		noecho();
		start_color();
		curs_set(0);
		init_pair(1,COLOR_BLACK,COLOR_YELLOW);
		init_pair(2,COLOR_BLACK,COLOR_CYAN);
}

void printFrame(){
		attrset(COLOR_PAIR(1));
		for(int i=0; i<height; i++)
		for(int j=0; j<width; j++){
				if(i==0 || i==height-1 || j==0 || j==1 || j==width-1 || j==width-2)
						mvaddch(offsetY+i, offsetX+j, 'X');
				else if(i%2==0 && (j%4==0 || j%4==1))
						mvaddch(offsetY+i, offsetX+j, 'X');
		}
}

void printObj(int ax, int ay, int a){
		attrset(COLOR_PAIR(1));
		switch(a){
				case 0:
					/*描写*/
					break;
				case 1:
					break;
				case 3:
					mvprintw(ay,ax,"/==\\");
					mvprintw(ay+1,ax,"vvvvv");
		sleep(3);
					break;
				case 4:
					break;
				default:
					break;
		}
}

void set_bomb(){}

int moveChara(){
		attrset(COLOR_PAIR(2));
		char c=(char)getch();
		switch(c){
				case 'w':
					y-=2;
					break;
				case 's':
					y+=2;
					break;
				case 'a':
					x-=4;
					break;
				case 'd':
					x+=4;
					break;
				case 'j':
					set_bomb(x,y);
					break;
				case 'q':
					return 1;
		}
		//move(y,x);
		mvprintw(y,x,"++++");
		mvprintw(y+1,x,"++++");
		return 0;
}

int to_Gpos(int Spos){
		int re=Spos*2;
		if(re>=width || re>=height)return -1;
		return re;
}

void bomb(int bx, int by){
}

int refreshAll(){
		clear();  // clear all
		printFrame();  // print frame
		if(moveChara()==1)return 1;
		return 0;
}

int main(){
		init();
		while(1){
				if(moveChara()==1)break;
		}
		endwin();
		//delscreen();
		return 0;
}
