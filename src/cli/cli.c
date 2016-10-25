//#include <mainDisplay.h>


void foo(int a){
		/* 通信 */
		refreshAll(metadata, bomb, player, wall); // 画面の再描写
}

int main(){
		
		/* 1/64秒に一回foo()が割り込み */
		while(1){
				pause();
				if(end_flag==1)break;
		}
		return 0;
}

