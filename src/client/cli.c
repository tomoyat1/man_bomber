#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#define BUFS     600
#define ERROR    -1
#define PORTn    10040

main(argc, argv)
  int argc;
  char *argv[];
{
  char buf[BUFS];
  int msglen;
  int al_msglen=0;
  int sd;
  struct sockaddr_in server;

  struct sockaddr_in client;

  int scktlen;
  struct hostent *hp;
  int i;
  int count=0;

  if(argc != 3){
    fprintf(stderr, "");
    exit(1);
  }

  /*ソケット作成*/
  if(( sd = socket(AF_INET, SOCK_STREAM, 0) == ERROR){
      perror("client: socket");
      exit(1);
    }

  bzero((char *)&client, sizeof(client));
  /*クライアントプロセスのソケットアドレス情報の設定*/

  client.sin_family = AF_INET;     /*ソケットをインターネットで使用する */
  client.sin_addr.s_addr = INADDR_ANY;     /*自分自身のIPアドレスを設定 */
  client.sin_port = htons(PORTn);                    /*ポート番号を設定 */
  scktlen = sizeof(client);              /*ソケットアドレス情報の長さを得る */
  bzero((char *)&server, sizeof(server));

  /*サーバプロセスのソケットアドレス情報の設定*/
  server.sin_family = AF_INET;      /*ソケットをインターネットで使用する*/
                                              /*サーバのIPアドレスの獲得*/
  if(( hp = gethostbyname(argv[1])) == NULL ){
      /*引数で与えられたホスト名が存在しなければエラーメッセージを出す．*/
    fprintf(stderr, "*ERROR: %s unknown host. \n", argv[1]);
    exit(1);                                                      /*終了*/
  }

                                         /*獲得したIPアドレスを設定する*/
  bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
                          
  server.sin_port = htons(atoi(argv[2]));     /*サーバのポート番号を設定*/
  len = sizeof(server);               /*ソケットアドレス情報の長さを得る*/




  /* 2) サーバプロセスへの接続要求の送信*/
  //ソケット作成失敗終了
    while(true){
      if( connect(sd, (struct sockaddr *)&server, scktlen) == ERROR){
	perror("client: connect");          
	exit(1);                            
      }
    //配列初期化
      memset(buf,0 ,sizeof(char) * BUFS);

      strcpy(buf,"");
      //fgets(buf, BUFS, "hoge");
      buf = "hoge"
      scktlen = strlen(buf);
      if(buf[len - 1] == '\n') {
	buf[len - 1] = '\0';
      }

      strcat(buf,"\r\n");
      msglen = strlen(buf);

      if(write(sd, buf, msglen+1) == ERROR){
	perror("client: write");
	exit(1 );
      }

      if( read( sd, buf, BUFSIZE ) == ERROR){
    perror("server: read");            /*読み出しが失敗したことを通知し*/
    exit(1);                                                     /*終了*/
  }
  printf("> ");
  for(i=0; i<= sizeof(buf); i++){  
                     /*サーバが送り返してきたメッセージを一文字づつ表示*/
    printf("%c", buf[i]);  
                                          /* 改行場合は,"> "を表示する */
    if(buf[i]== '\n'&& buf[i+1] != '\0'){
      printf("> ");    
    }
    else if(buf[i]== '\n'&& buf[i+1] == '\0'){
      break;
    }
  }
  close(sd);
  count++;
  sleep(3.0);
  if(count < 5) break;
 }

