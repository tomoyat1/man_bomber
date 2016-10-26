#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>

#include "man_bomber.h"
#include "man_bomber_config.h"
#include "send-data.h"
#include "recv-data.h"
#include "mainDisplay.h"

#define ERROR -1

void connect(int sd, struct sockaddr_in server, struct sockaddr_in client,
                 int scklen, struct hostent *hp);

main(int argc, char *argv[])
{
	struct metadata *data;
	struct player *pla[4];
	struct bomb *bo;
	struct wall *wa;
	
	int sd;
	struct sockaddr_in server;
	struct sockaddr_in client;

	int scklen;
	struct hostent *hp;
	
	int finish;

	/*First Connect*/
	int connect(int sd, struct sockaddr_in server, struct sockaddr_in client,
                 int scklen, struct hostent *hp)
	//data->id = -1;
	//

	init();
	while(1){
		if(argc != 3){
			fprintf(stderr, "Server IPaddress server_port\n");
			exit(1);
		}
		/*Socket Make*/
		if((sd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR){
			perror("client:socket");
			exit(1);
		}

		/* ClientProcess SocketAdress */
		bzero((char *)&client, sizeof(client));
		client.sin_family = AF_INET;
		client.sin_addr.s_addr = inet_addr("127.0.0.1");
		client.sin_port = htons(12345);
		scklen = sizeof(client);

		/* SeverProcess SocketAdress */
		bzero((char *)&server, sizeof(server));
		server.sin_family = AF_INET;
		if((hp = gethostbyname(argv[1])) == NULL){
			fprintf(stderr, "ERROR: %s unknown host. \n", argv[1]);
			exit(1);
		}

		/*IP Adress*/
		bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
		server.sin_port = htons(atoi(argv[2]));
		scklen =sizeof(server);

		if(connect(sd, (struct sockaddr *)&server,scklen) == ERROR){
			perror("client:connect");
			exit(1);
		}

		data_send(sd, data, &pla, &bo, &wa);
		data_recv(sd, data, *pla, *bo, *wa);
		finish = refreshAll(data, pla, bo, wa);

		if(finish == 1) break;
	}

	/* finish sign */
	
}

void connect(int sd, struct sockaddr_in server, struct sockaddr_in client,
		 int scklen, struct hostent *hp)
{
  if(argc != 3){
        fprintf(stderr, "Server IPaddress server_port\n");
        exit(1);
        }

        /*Socket Make*/
        if((sd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR){
        perror("client:socket");
        exit(1);
        }

        /* ClientProcess SocketAdress */
        bzero((char *)&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_addr.s_addr = inet_addr("127.0.0.1 ");
        client.sin_port = htons(12345);
        scklen = sizeof(client);

        /* ServerProcess SocketAdress */
        bzero((char *)&server, sizeof(server));
        server.sin_family = AF_INET;
        if((hp = gethostbyname(argv[1])) == NULL){
        fprintf(stderr, "ERROR: %s unknown host. \n", argv[1]);
        exit(1);
        }

        /*IP Adress*/
        bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
        scklen =sizeof(server);

        if(connect(sd, (struct sockaddr *)&server,scklen) == ERROR){
        perror("client:connect");
        exit(1);
        }
}
