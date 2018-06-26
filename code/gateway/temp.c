#define LIST 1000
#define MAX 1000000
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <ctype.h>
#include <json/json.h>

int socketFD;


// void createConn(int port){
// 	//crate a socket
// 	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	
// 	//check for errors
// 	if(socketFD <0){
// 		printf("Error: Can't make a socket\n");
// 		exit(1);
// 	}
	
// 	//set up attributes
// 	server.sin_family = AF_INET; //set connection to TCP
// 	server.sin_port = htons(port); //assigns port number
// 	server.sin_addr.s_addr = htons(INADDR_ANY);//sets addr to any interface
// 	int option = 1;
// 	setsockopt(socketFD,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &option,sizeof(option));
	
// 	//bind server information to socket
// 	if(bind(socketFD, (struct sockaddr *) &server, sizeof(server)) < 0){
// 		printf("can't bind\n");
// 		exit(1);
// 	}
// 	//start listening for clients, on the socket we made
// 	//and listen for up to 501 clients at a time
// 	listen(socketFD,501);
	
// }

struct sockaddr_in host_addr;
int main(int argc, char* argv[]){
	char *URL = "car-management-66420.appspot.com";
	struct hostent *host;
	//Get IP address from URL
	host=gethostbyname(URL);
	struct in_addr **address;
	if(host == NULL){ //send error 400 if bad request
		perror("HOST IS NULL");
		exit(0);
	}
	int success = 0;
	int port = 80;
	host_addr.sin_port=htons(port);
	host_addr.sin_family=AF_INET;
	//format IP address
	address = (struct in_addr **) host->h_addr_list;
	
	char dest[100];
	strcpy(dest,inet_ntoa(*address[0]));
	//address = (struct in_addr *) (host->h_addr);
	printf("connecting to: %s\n",dest);
	inet_pton(AF_INET,dest,&(host_addr.sin_addr));
	
	//make a socket and connect
	int sockfd1=socket(AF_INET,SOCK_STREAM,0);

	int connect2=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));

	if(connect2<0)
		printf("Error in connecting to remote server\n");
	

	char * string = "{\"EastRemote:1\" : \"open\"}";

	json_object *jobj = json_tokener_parse(string);
	const char *body = json_object_to_json_string(jobj);

	char head[MAX];
	char defaultmsg[MAX];
	//gateway - what we do
	//headers - just replies what i got
	sprintf(head,"%s%d\r\n%s%s", "POST http://car-management-66420.appspot.com/gateway HTTP/1.1\r\nContent-Type: text/plain\r\nContent-Length: ",
		 			(int) strlen(body),
		 			"Message_Type: Update",
		 			"\r\nAccept-Ranges: bytes\r\n\r\n");

	char *ztemp="doesthiswork?\r\n";
	sprintf(defaultmsg,"%s%s%d\r\n%s%s%s", "POST http://car-management-66420.appspot.com/headers HTTP/1.1\r\n",
					"Content-Type: text/plain\r\nContent-Length: ",
		 			(int) strlen(ztemp),
		 			"Message_Type: Update",
		 			"\r\nAccept-Ranges: bytes\r\n\r\n",
		 			ztemp);

	char msg[MAX];
	

	//printf("we have: [%s]\n",body);

	strncpy(msg,head,strlen(head));
	strncat(msg,body,MAX);
	strncat(msg,"\r\n",2);
	//send the data to the server
	printf("About to send [%s]",msg);
	printf("----------------------\n");
	if(send(sockfd1,msg,strlen(msg),0) < 0){
		printf("Failed to send\n");
	}
	char newbuff[MAX];
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd1, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	int n;
	while (1){
		if( (n=recv(sockfd1,newbuff,MAX,0) )< 0){
			
			perror("can't recv");
			break;
		}
		printf("Buffer is: %s\n",newbuff);
		
		//memset(newbuff,0,MAX);
	}
	


	printf("n is %i\n",n);
	
	close(sockfd1);


	return 0;
}