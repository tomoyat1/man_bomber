#include <curses.h>
#include <string.h>
//#include <stdio.h>
#include "man_bomber.h"

void init();
void printFld();
void printFldd();
void printObj();
void reWrite();
void gnrt4pics(int a);
void moveChara(char cc);

int main(){
		init();
		char c;

		printFld();
		while(1){
			c=(char)getch();
			if(c=='q')break;
		}

		endwin();
		return 0;
}


void init(){
		initscr();
		cbreak();
		noecho();
		start_color();
		init_pair(1,COLOR_RED,COLOR_YELLOW);
}

void printFld(){
		attrset(COLOR_PAIR(1));
		mvprintw(10,10,"Hello Curses World");
}

void printFldd(){
		attrset(COLOR_PAIR(1));
		mvprintw(30,30,"Hello Curses World2");
}

void gnrt4pics(int a){
		switch(a){
				case 0:
					/*描写*/
					break;
				case 1:
					break;
				case 3:
					break;
				case 4:
					break;
				default:
					break;
		}
}

void moveChara(char 
