#include <curses.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

#include "man_bomber.h"
#include "mainDisplay.h"

#define WIDTH 15
#define HEIGHT 9
#define offsetX 8
#define offsetY 10
#define FPS 33  // sleep time in milli sec

int player_id;
int client_id;
char c;
int kow[HEIGHT][WIDTH];
/* kow[][]=
	 0 何もなし
	 1 壊せない壁
	 2 壊せる壁
	 3 プレイヤー  
	 4 爆弾
*/

//以下デバッグ用
int bt = 0; // 起爆カウント
bool bomb_on = false;
int tick=0;


/* 破壊可能な壁をkowに登録・描画 */
void printWall(int wall_cnt, struct wall *wa){
		/* 初期化 */
		for(int i=0; i<HEIGHT; i++)
		for(int j=0; j<WIDTH; j++) kow[i][j]=0;

		for(int k=0; k<wall_cnt; k++) 
		for(int i=0; i<HEIGHT; i++)
		for(int j=0; j<WIDTH; j++) if(wa[k].y == i && wa[k].x == j) kow[i][j]=2;

		attrset(COLOR_PAIR(5));
		for(int i=0; i<HEIGHT; i++)
		for(int j=0; j<WIDTH; j++){
				if(kow[i][j]==2){
						attrset(COLOR_PAIR(5));
						printObj(j,i,'w');
				}
		}
}

void printPlayer(struct player *pl){
		for(int k=0; k<4; k++){
				if(pl[k].is_alive == 1){
						player_id=pl[k].id;
						printObj(pl[k].x, pl[k].y, 'p');
				}
		}
}

void init(){
		c='0';
		initscr();
		cbreak();
		noecho();
		start_color();
		timeout(FPS);
		curs_set(0);

		init_pair(1,COLOR_BLACK,COLOR_YELLOW);
		init_pair(2,COLOR_BLACK,COLOR_CYAN);
		init_pair(3,COLOR_RED,COLOR_BLACK);
		init_pair(4,COLOR_BLACK,COLOR_RED);
		init_pair(5,COLOR_BLACK,COLOR_GREEN);
		//init_pair(6,COLOR_BLACK,COLOR_GREEN);
}

/* 絶対座標に変換(フィールド左上が(0,0)) */
int toSpos(int p, char axis){
		if(axis=='x')return (p-offsetX)/4;
		else return (p-offsetY)/2;
}

/* 描画用座標に変換 */
int toGpos(int p, char axis){
		if(axis=='x')return p*4+offsetX;
		else return p*2+offsetY;
}

/* フィールドを描画，kowに破壊不可のブロック座標を追加 */
void printFrame(){
		/* 外枠のkow */
		for(int i=0; i<HEIGHT; i++){
				kow[i][0]=1; 
				kow[i][WIDTH-1]=1;
		}
		for(int i=0; i<WIDTH; i++){
				kow[0][i]=1;
				kow[HEIGHT-1][i]=1;
		}

		attrset(COLOR_PAIR(1));
		//int Gposx = toGpos(WIDTH, 'x');
		//int Gposy = toGpos(HEIGHT, 'y');
		int Gposx=WIDTH*4;
		int Gposy=HEIGHT*2;
		for(int i=-2; i<Gposy+2; i++)
		for(int j=-4; j<Gposx+4; j++){
				if(j<0 || (Gposx-1)<j || i<0 || (Gposy-1)<i){  // 外枠
						mvaddch(offsetY+i, offsetX+j, 'X');
				}else if(j%8>3 && i%4>1){  // 中のブロック
						mvaddch(offsetY+i, offsetX+j, 'X');
						if(i>0 && j>0 && (i/2)<=HEIGHT && (j/4)<=WIDTH)kow[i/2][j/4] = 1;
				}
		}
}


void printObj(int ax, int ay, char a){
		int ax_g = toGpos(ax, 'x');
		int ay_g = toGpos(ay, 'y');
		switch(a){
				case 'p':
					attrset(COLOR_PAIR(2));
					if(player_id==0){
							mvprintw(ay_g, ax_g, "(^^)");
							mvprintw(ay_g+1, ax_g, "/||\\");
					}else if(player_id==1){
							mvprintw(ay_g, ax_g, "(@@)");
							mvprintw(ay_g+1, ax_g, "/||\\");
					}else if(player_id==2){
							mvprintw(ay_g, ax_g, "^__^");
							mvprintw(ay_g+1, ax_g, "'||`");
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

void old_bomb_anime(int bbx, int bby){
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

void bomb_anime(int cnt, struct bomb *bo){
		for(int k=0; k<cnt; k++){
				if(bo[k].timer>96 || (64>bo[k].timer && bo[k].timer>32)){
						attrset(COLOR_PAIR(3));
						printObj(bo[k].x , bo[k].y , 'b');
				}else if((96>=bo[k].timer && bo[k].timer>=64) || (32>=bo[k].timer && bo[k].timer>0)){
						attrset(COLOR_PAIR(4));
						printObj(bo[k].x , bo[k].y , 'b');
				}else if(0>=bo[k].timer && bo[k].timer>-32){
						attrset(COLOR_PAIR(3));
						for(int i=1; i<bo[k].aoe; i++)
								printObj(bo[k].x+i , bo[k].y , 'r');
						for(int i=1; i<bo[k].aoe; i++)
								printObj(bo[k].x-i , bo[k].y , 'l');
						for(int i=1; i<bo[k].aoe; i++)
								printObj(bo[k].x , bo[k].y+i , 'd');
						for(int i=1; i<bo[k].aoe; i++)
								printObj(bo[k].x , bo[k].y-i , 'u');
				}
		}
}

int keyInput(char c, struct player *pl , struct bomb *bo){
		mvprintw(1,2,"(x,y)=(%d,%d)",pl[client_id].x,pl[client_id].y);
		mvprintw(2,2,"kow=%d",kow[pl[client_id].y][pl[client_id].x+1]);
		attrset(COLOR_PAIR(2));
		switch(c){
				case 'w':
					if(kow[pl[client_id].y-1][pl[client_id].x] != 0) return 0;
					//y-=1;
					pl[client_id].y--;
					break;
				case 's':
					if(kow[pl[client_id].y+1][pl[client_id].x] != 0) return 0;
					//y+=1;
					pl[client_id].y++;
					break;
				case 'a':
					if(kow[pl[client_id].y][pl[client_id].x-1] != 0) return 0;
					//x-=1;
					pl[client_id].x--;
					break;
				case 'd':
					if(kow[pl[client_id].y][pl[client_id].x+1] != 0) return 0;
					//x+=1;
					pl[client_id].x++;
					break;
				case ' ':
					if(pl[client_id].bombs<3){
							pl[client_id].bombs++;
							bo[0].x=pl[client_id].x;
							bo[0].y=pl[client_id].y;
							return 3;
					}else
							break;
				case 'q':
					end_flag = 1;
					return 0;
		}
		printObj(pl[client_id].x , pl[client_id].y , 'p');
		return 0;
}

void refreshAll(int ti){
		int ef;
		char c=(char)getch();
		clear();  // clear all
		printFrame();  // print frame
		//ef=keyInput(c);
		/* 爆弾の処理 */
		//if(ef==3) bomb_on=true;
		//if(bomb_on==true)bomb_anime(x,y);

		usleep(FPS);
}

void end(){
		endwin();
}

void new_main(struct metadata *me, struct bomb *bo, 
							struct player *pl, struct wall *wa){
		// bo[me->bomb_cnt-1].x
		clear();
		client_id = me->id;
		mvprintw(4,2,"client_id=%d",client_id);
		mvprintw(5,2,"tick=%d",tick);
		tick++;

		printFrame();
		printWall(me->wall_cnt, wa);
		printPlayer(pl);
		int kre = keyInput(c, pl, bo);
		//bomb_anime(int cnt, struct bomb *bo);

		c = (char)getch();
}
/*
int main(){
		init();

		struct sigevent tick_ev;
		timer_t tick_tm;
		struct itimerspec tick_spec;
		sigset_t alrm;

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
		tiemr_delete(alrm);
		endwin();
		return 0;
}
*/
