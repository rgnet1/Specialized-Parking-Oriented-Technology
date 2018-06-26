#define MAX 10000//128000
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>

char ip[50];
char msg[100];
//    For test 3
//char *temp = "1NorthRemote:1 ";
char *temp2 = "1EastRemote:2 ";
char *temp = "1EastRemote:1 ";
struct sockaddr_in server;
int makeConn(int port){
	//crate a socket
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	
	//check for errors
	if(socketFD <0){
		printf("Error: Can't make a socket\n");
		exit(1);
	}
	
	//set up attributes
	server.sin_family = AF_INET; //set connection to TCP
	server.sin_port = htons(port); //assigns port number
	
	struct hostent *host;
	struct sockaddr_in data_addr;
	strcpy(ip,"127.0.0.1");
	host = gethostbyname(ip);
					
	// check for errors
    if (host == NULL) {
        printf("Host error\n");
        exit(0);
    }
	
	inet_pton(AF_INET, ip, &data_addr.sin_addr);
	
	struct in_addr **address;
	address = (struct in_addr **) host->h_addr_list;
	char dest[100];
	strcpy(dest,inet_ntoa(*address[0]));
	inet_pton(AF_INET,dest,&(data_addr.sin_addr));
	
	//server.sin_addr.s_addr = htons(atoi(ip));//sets addr to any interface
	int option = 1;
	setsockopt(socketFD,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &option,sizeof(option));
	
	if(connect(socketFD, (struct sockaddr *)&server, sizeof(server))<0){
		perror("Failed to connect");
		exit(1);
	}
	return socketFD;
	
}

int main(int argc, char* argv[]){
	//strcpy(msg, temp);
	int port;
	if(argc ==2){
		port = atoi(argv[1]);
	}
	else{
		port= 1111;
	}
	
	if(port < 0){
		printf("Error, wrong port");
		exit(1);
	}
	
	if(argc ==4){
		//strcat(msg,argv[2]);
		sprintf(msg,"%s %s", argv[2],argv[3]);
	} else{
		printf("format: ./client <port> <1EastRemote:1> <status>\n");
		exit(1);
	}

	int sock = makeConn(port);
	//strcat(msg,argv[3]);
	printf("About to send [%s]\n",msg );
	if(send(sock,msg,strlen(msg),0) < 0){
		perror("Failed to send");
		}
		printf("Waiting to recv LED status...\n");
		char buff[MAX];
		memset(buff,0,MAX);
		if(recv(sock,buff,MAX,0) < 0){
			perror("Failed to recv");
		}
		printf("RECEIVED: [%s]\n",buff);
	

	return 0;
}
