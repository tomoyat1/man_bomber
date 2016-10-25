#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
//#include "man_bomber.h"

#define WIDTH 21
#define HEIGHT 13
#define offsetX 4
#define offsetY 10
#define xx 0
#define yy 0
#define FPS 15625  // sleep time in micro sec


int end_flag=0;
int x=(offsetX/2)+xx, y=(offsetY/2)+yy; 
int playerID=0;
int bt = 0; // 起爆カウント
bool bomb_on = false;
int kow[HEIGHT][WIDTH];
/* 0 何もなし
	 1 壊せない壁
	 2 壊せる壁
	 3 プレイヤー  
	 4 爆弾
*/

void setWall(){
		for(int i=0; i<HEIGHT; i++)
		for(int j=0; j<WIDTH; j++) kow[i][j]=0;
}

void init(){
		initscr();
		cbreak();
		noecho();
		start_color();
		curs_set(0);

		setWall();

		init_pair(1,COLOR_BLACK,COLOR_YELLOW);
		init_pair(2,COLOR_BLACK,COLOR_CYAN);
		init_pair(3,COLOR_RED,COLOR_BLACK);
		init_pair(4,COLOR_BLACK,COLOR_RED);
		init_pair(5,COLOR_BLACK,COLOR_GREEN);
}

int toSpos(int p, char axis){
		if(axis=='x')return p-(offsetX/2);
		else return p-(offsetY/2);
}

/* axis x or y */
int toGpos(int p, char axis){
		if(axis=='x')return p*4;
		else return p*2;
}

void printFrame(){
		attrset(COLOR_PAIR(1));
		int Gposx = toGpos(WIDTH, 'x');
		int Gposy = toGpos(HEIGHT, 'y');
		for(int i=-2; i<Gposy+2; i++)
		for(int j=-4; j<Gposx+4; j++){
				if(j<0 || (Gposx-1)<j || i<0 || (Gposy-1)<i){  // 外枠
						mvaddch(offsetY+i, offsetX+j, 'X');
						//kow[i/2][j/4] = 1;
				}else if(j%8>3 && i%4>1){  // 中のブロック
						mvaddch(offsetY+i, offsetX+j, 'X');
						kow[i/2][j/4] = 1;
				}
		}
}


void printObj(int ax, int ay, char a){
		int ax_g = toGpos(ax, 'x');
		int ay_g = toGpos(ay, 'y');
		switch(a){
				case 'p':
					attrset(COLOR_PAIR(2));
					if(playerID==0){
							mvprintw(ay_g, ax_g, "(^^)");
							mvprintw(ay_g+1, ax_g, "/||\\");
					}else if(playerID==1){
							mvprintw(ay_g, ax_g, "OOOO");
							mvprintw(ay_g+1, ax_g, "OOOO");
					}else if(playerID==2){
							mvprintw(ay_g, ax_g, "OOOO");
							mvprintw(ay_g+1, ax_g, "OOOO");
					}else{
							mvprintw(ay_g, ax_g, "OOOO");
							mvprintw(ay_g+1, ax_g, "OOOO");
					}
					break;
				case 'b':
							mvprintw(ay_g, ax_g, "/==\\");
							mvprintw(ay_g+1, ax_g, "\\==/");
					break;
				case 'r':
					mvprintw(ay_g,ax_g,"===}");
					mvprintw(ay_g+1,ax_g,"===}");
					break;
				case 'l':
					mvprintw(ay_g,ax_g,"{===");
					mvprintw(ay_g+1,ax_g,"{===");
					break;
				case 'd':
					mvprintw(ay_g,ax_g,"||||");
					mvprintw(ay_g+1,ax_g,"WWWW");
					break;
				case 'u':
					mvprintw(ay_g,ax_g,"MMMM");
					mvprintw(ay_g+1,ax_g,"||||");
					break;
				case 'w':
					mvprintw(ay_g,ax_g,"####");
					mvprintw(ay_g+1,ax_g,"####");
					break;
				default:
					break;
		}
}

void printWall(){
		attrset(COLOR_PAIR(5));
		for(int i=0; i<HEIGHT; i++){
				for(int j=0; j<WIDTH; j++){
						if(kow[i][j]==2)printObj(j,i,'w');
				}
		}
}

void bomb_anime(int bbx, int bby){
		mvprintw(2,2,"%d",bt);
		static int bx, by;
		if(bt==0) {
				bx=bbx, by=bby;
		}
		if(bt>=128){
				attrset(COLOR_PAIR(3));
				printObj(bx,by,'b');
				for(int i=1; i<=3; i++){
						printObj(bx+i, by, 'r');
				}
				for(int i=1; i<=3; i++){
						printObj(bx-i, by, 'l');
				}
				for(int i=1; i<=3; i++){
						printObj(bx, by+i, 'd');
				}
				for(int i=1; i<=3; i++){
						printObj(bx, by-i, 'u');
				}
				bt++;
		}else if(bt<32 || (64<=bt && bt<96)){
				attrset(COLOR_PAIR(3));
				printObj(bx,by,'b');
				bt++;
		}else{
				attrset(COLOR_PAIR(4));
				printObj(bx,by,'b');
				bt++;
		}
		if(bt==160) {
				bomb_on = false;
				bt=0;
		}
}

int keyInput(char c){
		mvprintw(1,2,"(x,y)=(%d,%d)",x,y);
		attrset(COLOR_PAIR(2));
		switch(c){
				case 'w':
					//if(kow[y-1][x] != 0) return 0;
					y-=1;
					break;
				case 's':
					//if(kow[y+1][x] != 0) return 0;
					y+=1;
					break;
				case 'a':
					//if(kow[y][x-1] != 0) return 0;
					x-=1;
					break;
				case 'd':
					//if(kow[y][x+1] != 0) return 0;
					x+=1;
					break;
				case ' ':
					return 3;
				case 'q':
					end_flag = 1;
					return 0;
		}
		printObj(x,y,'p');
		return 0;
}

int to_Gpos(int Spos){
		int re=Spos*2;
		if(re>=WIDTH || re>=HEIGHT)return -1;
		return re;
}

void bomb(int bx, int by){
}

void setBombPos(){
}

void refreshAll(int ti){
		int ef;
		char c=(char)getch();
		clear();  // clear all
		printFrame();  // print frame
		ef=keyInput(c);
		/* 爆弾の処理 */
		if(ef==3) bomb_on=true;
		if(bomb_on==true)bomb_anime(x,y);

		usleep(FPS);
}

int main(){
		init();

		struct sigevent tick_ev;
		timer_t tick_tm;
		struct itimerspec tick_spec;
		sigset_t alrm;
		/* tick timer */
		tick_ev.sigev_notify = SIGEV_SIGNAL;
		tick_ev.sigev_signo = SIGALRM;
		timer_create(CLOCK_REALTIME, &tick_ev, %tick_tm);
		tick_spec.it_interval.tv_sec = 0;
		tick_spec.it_interval.tv_nsec = 15625000;
		tick_spec.it_value.tv_sec = 1;
		tick_spec.it_value.tv_nsec = 0;
		timer_settime(tick_tm, 0, &tick_spec, &tick_spec); 

		signal(SIGALRM, refreshAll);
		while(1){
				pause();
				if(end_flag==1)break;
		}
		/*
		while(1){
				if(refreshAll()==1)break;
		}
		*/
		tiemr_delete(alrm);
		endwin();
		return 0;
}
