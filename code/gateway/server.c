
#include "server.h"


//thread counter mutex
pthread_mutex_t mutexHost = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AEmutex = PTHREAD_MUTEX_INITIALIZER;

//Shared memory
int checkIn[MCCount];
char lot[MCCount][20];
char statusF[MCCount][20];

//HTTP message header string constants
char *postFirstLine = "POST http://car-management-66420.appspot.com/gateway HTTP/1.1\r\n";
char *getFirstLine = "GET http://car-management-66420.appspot.com/gateway HTTP/1.1\r\n";
char *contentLine = "Content-Type: text/plain\r\nContent-Length: ";
char *OGmessageType = "Message_Type: Update\r\nAccept-Ranges: bytes\r\nGateway: ";
char staticParkingLot[25];
char staticGatewayNum[1000];
//Shared 
int timeFlag = 0; 
int sentGetReq= 0;
int endGetReq = 0;

// Socket variables
int socketFD;
struct sockaddr_in server, client;

// Thread variables
int threadCount, threadWait;



int main(int argc, char* argv[]){
	
	// getting port from user input
	// and make a listening socket
	int connected;
	int port, i;
	if(argc == 4){
		strncpy(staticParkingLot,argv[2],strlen(argv[2]));
		strncat(staticParkingLot,":",1);
		port = atoi(argv[1]);
		strcpy(staticGatewayNum,OGmessageType);
		strcat(staticGatewayNum,argv[3]);
		strcat(staticGatewayNum,"\r\n\r\n");
	} else{
		printf("Usage: ./server <port> <ParkingLot> <gatewayNum>\n");
		exit(1);
	}
	createConn(port);

	// Establish that no sensor
	// checked in
	for(i = 0; i<MCCount;++i){
		checkIn[i] = -1;
	}

	// Netowrking variable
	int addrSize = sizeof(struct sockaddr_in);


	printf("Waiting for clients...\n");

	// Data buffer
	char buffer[MAX];

	// Array of thread Data to
	// handle tMAX threads at a time
	tdata threadData[tMAX];
	for(i = 0; i< tMAX;++i){
		threadData[i].buffer = malloc(sizeof(char) *MAX);
		memset(statusF[i],0,MCCount);
	}

	// Declare tMAX of threads
	pthread_t t[tMAX];

	//Initalize variables
	threadCount=0;
	int threadIndex = 0;
	threadWait = 0;

	//sleep while waiting to accept connections
	while((connected = accept(socketFD,
							 (struct sockaddr *)&client,
							 (socklen_t*)&addrSize)) >0){
		
		//reset buffer
		memset(&buffer,0,MAX);

		// make sure thread index gets reset
	    // back to 0
		if(threadIndex >= tMAX){
			printf("***** REACHED MAX******** %d\n",threadIndex);
			threadIndex = 0;
		}
		printf("Therad INdex = %i\n",threadIndex);

		// If we have the maximum number of 
		// threads running wait for a thread
		// to complete
		if(threadCount >=tMAX){
			printf("stuck in here\n");
			if(threadWait  == tMAX){
				threadWait = 0;
			}
			pthread_join(t[threadWait],NULL);
			threadIndex = threadWait;
			++threadWait;
			if(threadWait == tMAX){
				threadWait = 0;
			}
			--threadCount;
					
		}// end threadCount >= tMAX
		
		// save the socket of the sensor that just
		// came in
		threadData[threadIndex].socketFD = connected;


		//------------Set up Time Out for recving from SENSOR--------------------
		struct timeval tvSensor;
		tvSensor.tv_sec = 540;// 9 min
		tvSensor.tv_usec = 0;
		setsockopt((*threadData).socketFD, SOL_SOCKET, SO_RCVTIMEO, 
			       (const char*)&tvSensor, sizeof tvSensor);
		//------------Done setting up Time Out for App Engine Socket----------------

		// Create a Thread
		if(pthread_create(&t[threadIndex],NULL,getData, &threadData[threadIndex]) !=0){
			perror("error making thread");
		}else{ //created thread successfully
			printf("Made successfully\n");

			// increment variables 
			pthread_mutex_lock(&mutex);
				++threadCount;
				++threadIndex;
			pthread_mutex_unlock(&mutex);
			


			//       TimerFlag
			//
			// 0   if we are waiting for msg type
			// 1   if we checking for hearbeat msgs
			// 2   if we have an event/heartbeat mesg

			// If flag is 0, wait
			while(timeFlag == 0){
				//busy while loop
			}

			// Check for heartbeat messages
			// if it has been 15 minutes
			if(timeFlag == 1){
				checkInNow();
				printf("Waiting for Sensors...\n");
			}

			// Otherwise, its and Event message
			// so we reset timeFlag back to 0
			else if(timeFlag == 2){
				timeFlag = 0;
			}
			
		}//end else

	}// end while(accept())

	//   Free malloced Data
	//  (but we never get here)
	// for(i = 0; i< tMAX;++i){
	// 	free(threadData[i].buffer);
	// }
	return 0;
} //end main()


//       getData()
//
// @param: tData
//
// This function is called whenever a new sensor
// has data to send. This function gets the data
// from the sensors and decides what to do with it
void* getData(void* tData){
	printf("NEW THREAD\n");
	tdata *threadData = (tdata*) tData;
	memset((*threadData).buffer,0,MAX);	
	
	// Get string: "changeBit ParkingLot:ParkingSpot Status"
	// from the sensor
	if(recv((*threadData).socketFD,(*threadData).buffer, MAX,0) <= 0){
		perror("can't recv from micro controller");
		pthread_exit(NULL);
	}
	printf("      GOT FROM Micro Controller: \n[%s]\n",(*threadData).buffer);
	
	// Send/recv timeout after 3 attempts
	int errCount;



	//if the timer program sent a message, the buffer
	//would be "timer".
	//
	// This means we want to set the public timeFlag
	// to 1 so I can check to make sure all sensors 
	// checked in
	if(strcmp((*threadData).buffer,"timer") == 0){
		timeFlag = 1;
		pthread_exit(NULL);
	}else{
		//set flag to 2 if we have an Event message or 
		//heartbeat message
		timeFlag = 2;
	}

	// If we got nothing, exit thread
	if(strlen((*threadData).buffer) ==0){
		printf("exiting thread\n");
		pthread_exit(NULL);
	}

	//             buffer format:
	// <Eventbit><ParkingLot>:<spot> <event>

	// Make copies of received data
	char bufferCpy[MAX];
	char cpy[MAX];

	memset(cpy,0,MAX);
	strcpy(bufferCpy,(*threadData).buffer);	
	strcpy(cpy,(*threadData).buffer);

	// Split up mesage into an 
	// array
	char *cpy2 = cpy;
	char *p = strtok (++cpy2, " ");
	char *array[2];
	int i = 0;
	while (p != NULL){
  	  array[i++] = p;
  	  p = strtok (NULL, " ");
	}



	// Check for An event message
	// (Changedbit = 1)
	if(bufferCpy[0] == '1'){

		// Look for spot in our shared memory
		// and insert it if it doesn't exist
		for(i = 0; i< MCCount;++i){
			if(strcmp(lot[i],array[0]) == 0){
				break;
			}
			else if(strlen(lot[i]) == 0){
				strcpy(lot[i],array[0]);
				printf("LOT FILLED IS: [%s]\n",lot[i]);
				break;
			}
			
		} // end for()

		// We need to send a get message,
		// so turn on the GET flag
  		int sendGETFlag = 1;

  		if(strstr(array[1],"open") !=0){
  			printf("setting get flag to 0\n");
  			sendGETFlag = 0;
  		}

  		//Set up JSON data in the following format:
  		// {"<ParkingLot>:<Spot>","<status>"}
  		char string[MAX];
  		memset(string,0,MAX);
  		sprintf(string,"{\"%s\" : \"%s\"}",array[0],array[1]);

		
  		//connect to app engine (attempt up to 3 times)
		int socketAE;
		int x = 0;
		while(1){
			if((socketAE = connToAppEngine())<0 ){
				++x;
				if(x >3){
					close((*threadData).socketFD);
					printf("Can't connect to app engine\n");
					pthread_exit(NULL);
				}
			}else
				break;
		}

		//-------------Set up HTTP Message--------------------
		char msg[MAX];
		setUpPostMsg(msg, string);
		//----------Done Setting up HTTP Message--------------

		
		printf("--------------about to send:\n%s",msg);

		//Send spot name and spot status (attempt 3 times)
		x = 0;
		while(1){
			if(send(socketAE, msg,strnlen(msg,MAX),0) < 0){
				perror("Can't send to App engine");
				if(++x> 3){
					close((*threadData).socketFD);
					close(socketAE);
					pthread_mutex_lock(&mutex);
					--threadCount;
					pthread_mutex_unlock(&mutex);
					pthread_exit(NULL);
				}
			}else
				break;
		}
		char newbuff[MAX];

		//------------Set up Time Out for recving from App Engine--------------------
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		setsockopt(socketAE, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
		//------------Done setting up Time Out for App Engine Socket----------------

		// expect "status ok" for verification upon POST request
		// recv up to 3 times
		int n;
		x = 0;
		while(1){
			if( (n=recv(socketAE,newbuff,MAX,0) )< 0){
				perror("Can't recv confirmation AE got our req");
				if(++x > 3){
					close((*threadData).socketFD);
					close(socketAE);
					pthread_mutex_lock(&mutex);
					--threadCount;
					pthread_mutex_unlock(&mutex);
					pthread_exit(NULL);
				}
			} else
				break;
		}
	
		//printf("******Buffer is: ****\n%s",newbuff);
		
		//extract data app engine sent back from the
		//entire HTTP message
		char *ptr = strstr(newbuff,"\r\n\r\n");
		ptr+=4;
		//printf("=========POST response========\n[%s]\n",ptr);


		//         First Thread
		//
		// If we need to send a get request &&
		// A GET request has not ben sent
		if(sendGETFlag && sentGetReq == 0){

			//tell all other threads I'm talking to AE
			sentGetReq = 1;

			//actually talk to AE
			pthread_mutex_lock(&AEmutex);
			talkToAppEngine(threadData, socketAE, array,newbuff);
			pthread_mutex_unlock(&AEmutex);

		}// end  if(sendGETFlag)

		//          Not the First Thread
		//
		// If we need to send a GET request &&
		// a get request is already being sent
		else if(sendGETFlag && sentGetReq == 1){
			waitForAppEngine(threadData, array);
			
		}
		// If we get ACK ("ok") from cloud, send it
		// back to the sesor
		else{
			
			if(strstr("ok",ptr) == 0){
				errCount =0;
				// send OK up to 4 times
				while(1){
					if(send((*threadData).socketFD,"ok",2,0) < 0){
						perror("Can't send LED status");
						if(++errCount > 3){
							close((*threadData).socketFD);
							close(socketAE);
							--threadCount;
							pthread_exit(NULL);
						}
					} else
						break;
				}
			}
			else{
				printf("did not get status ok from sending \"open\" \n");
			}

			//decrement threadCount
			pthread_mutex_lock(&mutex);
			--threadCount;
			pthread_mutex_unlock(&mutex);

		}

		//close connection to the App Engine
		close(socketAE);
		
	} //end if changeBit = 1


	// We got heartBeat or "open", send ACK to sensor
	// and note that the parking space checked in
	else{
		errCount = 0;

		//send ACK back to the sensor up to 3 times
		while(1){
			printf("ABOUT TO send ACK\n");
			if(send((*threadData).socketFD,"ACK",3,0) < 0){
				perror("Can't send LED status");
				if(++errCount > 3){
					close((*threadData).socketFD);
					--threadCount;
					pthread_exit(NULL);
				}
			}else
				break;
		}

		//extract number
		char *num = strtok(array[0],":");
		num = strtok(NULL,":");
		printf("Parking lot number is [%s]\n",num);
		int conv = atoi(num);

		// note that this sensor checked in
		checkIn[conv-1] = 1;

		pthread_mutex_lock(&mutex);
		--threadCount;
		pthread_mutex_unlock(&mutex);

	}//end ACK else
	
	printf("Waiting for Sensors...\n");
	pthread_exit(NULL);
}


//            waitForAppEngine()
//
// @param: threadData, array
//
// This function takes in the thread data and 
// array containing spot information. It waits
// for the App Engine to respond with this threads'
// sensor status
void waitForAppEngine(tdata *threadData, char *array[2]){
	int found = 0;
	int x;
	int b;
	int errCount;
	//stay here until we find the data
	while(found == 0){
		//check every 500 micro seconds
		usleep(50);
		
		//when the first thread recieves data, sendGetRequest is set
		//back to 0. This means we are ready to send data to other threads
		if(sentGetReq  == 0){
			
			for(b = 0; b<tMAX;++b){
				if(strlen(statusF[b]) == 0){
					break;
				}
				printf("   statusF[%i] =[%s]",b,statusF[b]);
				printf("   lot[%i] =[%s]\n",b,lot[b]);
			}
			int x; 
			for(x = 0; x<tMAX;++x){
				printf("lot is: [%s]  array is [%s]\n",lot[x],array[0]);
				if(strcmp(lot[x],array[0]) ==0 || ( strlen(lot[x]) == 0 && endGetReq ==-1)){
					found = 1;
					printf("FOUND IT [%s]   [%s]\n",lot[x],array[0]);
					if(endGetReq == -1){
						strcpy(statusF[x],"illegal");
					}
					if(strlen(statusF[x]) !=0){
						errCount =0;
						while(1){
							if(send((*threadData).socketFD,statusF[x],strlen(statusF[x]),0) < 0){
								perror("Can't send LED status from FOUND IT");
								if(++errCount>4){
									printf("Cant send\n");
									break;
								}

							} else
								break;
						}

					memset(statusF[x],0,20);
					break;
					} else{
						found = 0;
						break;
					}

				}
			}
			// If we didn't find our spot, that means
			// a different spot was verfied, 
			// so set found = 0 and keep waiting
			if(!found){
				printf("&&&&&&&&&&&&&&&&&&&Not Found, sentGetReq is %i\n",sentGetReq);
				//continue to send get req
				sentGetReq = 1;

				// set found = 0
				found = 0;

			}
			// If we found our spot, subtract the thread
			// count and clear the status and subtract
			// thread count
			else{
			 	usleep(10);
				pthread_mutex_lock(&mutex);
					--threadCount;
				pthread_mutex_unlock(&mutex);
				if(threadCount == 0){

					//reset status of all lots
					for(x = 0; x <MCCount;++x){
						//memset(lot[x],0,20);
						if(strlen(lot[x]) == 0){
							break;
						}
						//printf("str lent is: %i",(int) strlen(lot[x]));
						memset(statusF[x],0,20);
						printf("EMPTIED LOT: [%s]   Status [%s]\n",lot[x],statusF[x]);
					}
				}else{
					//DEBUG: printf("******** NOT clearing status******* %i\n",threadCount);
				}
			} // end  if(found)   (else)
		}// end if(sentGETreq)
	}//end while(found == 0)
} //end waitForAppEngine function


//           createConn()
// @param: port
//
// This function establishes a connection at
// the specified port
void createConn(int port){
	//crate a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	
	//check for errors
	if(socketFD <0){
		printf("Error: Can't make a socket\n");
		exit(1);
	}

	//set connection to TCP
	server.sin_family = AF_INET; 

	//assigns port number
	server.sin_port = htons(port);

	//sets addr to any interface
	server.sin_addr.s_addr = htons(INADDR_ANY);
	int option = 1;
	setsockopt(socketFD,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &option,sizeof(option));
	
	//bind server information to socket
	if(bind(socketFD, (struct sockaddr *) &server, sizeof(server)) < 0){
		printf("can't bind\n");
		exit(1);
	}

	//start listening for clients, on the socket we made
	//and listen for up to 501 clients at a time
	listen(socketFD,501);
	
}

//                 checkInNow()
//
// This function checks to ensure sensors checked in with
// heartbeat messages
void checkInNow(){
	int i;
	//      Check In status:
	//
	// -1:     MC did no Check in
	//
	//  0:     MC did check in
	//
	// >=1:    MC did not Check in
	//         multiple times
	int socket = connToAppEngine();
	for(i =0;i< MCCount;++i){

		// If sensor did not check in
		if(checkIn[i] == -1){ 
			//printf("Sensor %i did not check in\n",i);
			checkIn[i] = 1;
		}
		// If sensor checked in, reset back to 0
		else if(checkIn[i] == 0){
			
			checkIn[i] = -1;
		}
		// If sensor didn't check in multipl times
		else if(checkIn[i] > 0){
			//printf("Non check in vale is %i at spot %i\n",checkIn[i],i);
			checkIn[i]+=1;
			//printf("result is: %i\n",checkIn[i]);
			if(checkIn[i]  == 4){
				//printf("too much error\n");
				
				char temp[MAX];
				char sending[MAX];
				strncpy(sending,staticParkingLot, strlen(staticParkingLot));
				char lotNum[3];
				sprintf(lotNum,"%d",(i+1));
				strncat(sending,lotNum,strlen(lotNum));
				strcat(sending," broken");
				int x = 0;
				while(1){
					printf("about to send [%s] to spot [%i]",sending,i);
					if(send(socket,sending,strlen(sending),0) < 0){
						perror("Can't send \"broken\" to AE\n");
						if(++x > 3){
							break;
						}
					}else{
						//printf("Sent successfully\n");
						//recv(socket,temp,MAX,0);
						//printf("got [%s]",temp);
						break;
					}
				}
				printf("Done sending broken\n");
				memset(sending,0,MAX);
				
			}
		}
	} // end itteration through list of sensors
	close(socket);
} //end checkIn() function


//            int  connToAppEngine()
//
// @return: int - socket used to talk to the AE
//
// This function is called whenever we want to make a
// connection with the Google Cloud App Engine
int connToAppEngine(){
	char *URL = "car-management-66420.appspot.com";
	struct hostent *host;
	struct sockaddr_in host_addr;
	printf("Abut to get host by name\n");

	// Get IP address from URL.
	//
	// (mutex lock because gethostbyname() is not
	// thread safe).
	pthread_mutex_lock(&mutexHost);
	host=gethostbyname(URL);
	pthread_mutex_unlock(&mutexHost);


	struct in_addr **address;
	if(host == NULL){ //send error 400 if bad request
		perror("HOST IS NULL");
		exit(0);
	}

	printf("Host is not null\n");
	host_addr.sin_port=htons(80);
	host_addr.sin_family=AF_INET;
	//format IP address
	address = (struct in_addr **) host->h_addr_list;
	
	char dest[100];
	strcpy(dest,inet_ntoa(*address[0]));
	//address = (struct in_addr *) (host->h_addr);
	printf("connecting to: %s\n",dest);
	inet_pton(AF_INET,dest,&(host_addr.sin_addr));
	printf("About to make socket\n");
	//make a socket and connect
	int sockfd1;
	if((sockfd1=socket(AF_INET,SOCK_STREAM,0) ) < 0){
		perror("Failed to make socket to app engine");
		//exit(1);
	}
	int option = 1;
	setsockopt(sockfd1,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &option,sizeof(option));

	int connect2;
	if((connect2=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr)))<0){
		perror("Error in connecting to remote server\n");
		exit(1);
	}
	printf("Done connecting\n");
	return sockfd1;


}


//        talkToAppEngine()
//
// @param: threadData, socketAE, array, newbuff
//
// formats and sends an HTTP GET request to send
// and recieve data from the Google Cloud App Engine.
void talkToAppEngine(tdata *threadData, int socketAE, char *array[2], char newbuff[MAX]){
	int first =1;
	//counter variable
	int x;

	int foundIt=0;

	int doneFlag= 1;

	//number of bytes received by send() or recv()
	int n;

	//send/recv counter we use to time out 3 times
	int errCount;
	//while(1){
		char getMSG[MAX];
		sprintf(getMSG,"%s%s%d\r\n%s",getFirstLine,
							  		contentLine,
	 								0,
	 								staticGatewayNum);
	while(doneFlag){
		//send get message to app engine
		//printf("\n-------------ABOUT TO SEND GET REQUEST------------------: \n[%s]\n",getMSG);
		endGetReq = 0;
		//send Get request and try to get response until 10 min
		while(1){
			if(threadCount == 0){
				return;
			}
			if(send(socketAE,getMSG,strlen(getMSG),0) < 0){
				perror("Can't send GEt request");
			}
			memset(newbuff,0,MAX); 
			printf("-------------Done sending get request to cloud-------------threadCount: %i\n",threadCount);
			if( (n=recv(socketAE,newbuff,MAX,0) )< 0){
				perror("Can't recv from get request");
				}
				//status: none = nothing to verify
			//printf("done recving\n");
			//printf("[%s]\n",newbuff);
			if(strstr(newbuff,"ok") !=0 ){ //|| strstr(newbuff, "illegal") != 0
					break;
				}
				else{ 
					//printf("going to sleep \n");
					usleep(5000000); //5 seconds
					//printf("Waiking up\n");
				}
				//120 is 10 min
			if(++endGetReq ==GETTIMEOUT){
				endGetReq = -1;
				break;
			}
				

		}//end while loop

		--first;
		//DEBUG:
		//printf("-------------GOT BACK FROM GET REQUEST------------ \n[%s]",newbuff);
		
		//If we timed out (10 min without verificatin)
		//tell spot and app engine the spot is illegal
		if(endGetReq== -1){
			
			handleAppEngineTimeOut(threadData,socketAE,array);
		}

		//extract status from HTTP result
		char *status = strstr(newbuff,"\r\n\r\n");
		status+=4;
		
		
		//the status is in the correct format
		printf("\n");
		//printf("about to send [%s]\n",status);

		//split the status' by lot and statusF
		split(status,statusF,lot);

		//---------------Print populated StatusF and Lot --------------------
		// int b;
		// for(b = 0; b<tMAX;++b){
		// 	if(strlen(statusF[b]) == 0){
		// 		break;
		// 	}
		// 	printf("   statusF[%i] =[%s]",b,statusF[b]);
		// 	printf("   lot[%i] =[%s]\n",b,lot[b]);

		// }
		//----------------End printing StatusF and Log------------------------
		if(foundIt == 0){
			printf("thread 1, foutn It is 0 [%s]\n",array[0]);
			for(x = 0; x<tMAX;++x){
				printf("11111111lot is: [%s]  array is [%s]\n",lot[x],array[0]);
				if(strlen(lot[x]) == 0){
					break;
				}
				if(strcmp(lot[x],array[0]) ==0 && strlen(statusF[x])>0){
					printf("FOUND IT\n");
					foundIt = 1;
					int errCount = 0;
					while(1){
						if(send((*threadData).socketFD,statusF[x],strlen(statusF[x]),0) < 0){
								perror("Can't send LED status from FOUND IT");
								if(++errCount > 3){
									close((*threadData).socketFD);
									close(socketAE);
									pthread_exit(NULL);
								}
						}else{
							printf("Sent [%s] successfully\n",statusF[x]);
							pthread_mutex_lock(&mutex);
							--threadCount;
							if(threadCount == 0){
								int z;
								for(z = 0; z <tMAX;++z){
									//memset(lot[x],0,20);
									if(strlen(lot[z]) == 0){
										break;
									}
									memset(statusF[z],0,20);
									printf("EMPTIED LOT: [%s]   Status [%s]\n",lot[z],statusF[z]);
								}
							}else{
								printf("*********** NOT clearing stuff********** %i\n",threadCount);
							}
							pthread_mutex_unlock(&mutex);
							if(threadCount <1){
								doneFlag = 0;
								break;
							}
							else{
								doneFlag = 1;
								break;
							}
						}
					}
					//reset status'
					memset(statusF[x],0,tMAX);
					break;
				} else{ //If we did not get the spot
					if(threadCount <1){
						doneFlag = 0;
					}
				}
				
			}
		}// end if(foundIt)
		sentGetReq = 0;
		if(x == tMAX || !foundIt){
			printf("Couldn't find it, T1\n");
			
			//forsure not done, keep flag 1
			doneFlag = 1;
		}
		//printf("\n");

		//set the flag to 0 since we are done sending get req
		//this allows the next thread to fire to send
		//get reqests
		if(threadCount < 1){
			sentGetReq = 0;
			doneFlag = 1;
			break;
		}
		else{
			printf("Couldn't find it\n");
			sentGetReq = 0;
			//forsure not done, keep flag 1
			doneFlag = 1;
		 	printf("Thrad count efasfasf is: %i\n",threadCount);
		 	
		 }
	}

}


//        handleAppEngineTimeout()
//
// @param: tdata, socketAE, array[2]
//
// If the App Engine doesn't respond within a reasonable
// time, this function is called.
void handleAppEngineTimeOut(tdata *threadData, int socketAE, char *array[2]){
	//keep track of number of vars
	int errCount;

	//index incrementor
	int x = 0;

	//itterate through eall spots and send "illegal"
	for(x = 0; x<tMAX;++x){
		printf("lot is: [%s]  array is [%s]\n",lot[x],array[0]);

		//Make sure strint is not in the list
		//if(strcmp(lot[x],array[0]) ==0 || strlen(lot[x]) == 0){
		strcpy(statusF[x],"illegal");
		//DEBUG: printf("FOUND IT [%s]   [%s] [%s]\n",lot[x],array[0],statusF[x]);

		//send to sensor up to 3 times
		int tempx = 0;
		while(1){
			if(send((*threadData).socketFD,statusF[x],strlen(statusF[x]),0) < 0){
					perror("Can't send LED status from FOUND IT");
					if(++tempx > 3){
						close((*threadData).socketFD);
						close(socketAE);
						pthread_exit(NULL);
					}
			}else
				break;
		}

		

		//--------Formate HTTP Post MSG---------------------
		char msg[MAX];
		char temp[MAX];
		memset(temp,0,MAX);
		sprintf(temp,"{\"%s\" : \"%s\"}",lot[x],"illegal");		
		setUpPostMsg(msg, temp);
		//---------------------------------------------------

		//send to App Engine up to 3 times
		errCount = 0;
		while(1){
			printf("about to send illagal to app engine: [%s]\n",msg);
			if(send(socketAE,msg,strlen(msg),0) < 0){
					perror("Can't send LED status from FOUND IT");
					if(++errCount > 3){
						close((*threadData).socketFD);
						close(socketAE);
						pthread_exit(NULL);
					}
			}else
				break;

		}
		memset(statusF[x],0,20);
		printf("Thread Count: %i\n",threadCount );
		if(threadCount ==x+1){
			printf("BREAKING\n");
			break;
		}
	}
	sentGetReq = 0;
	close(socketAE);
	close((*threadData).socketFD);
	printf("Waiting for sensors...\n");

	pthread_mutex_lock(&mutex);
	--threadCount;
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}


//        setUpPostMsg()
//
// @param: msg, string
//
// Takes a buffer for the final msg ressult and 
// a string body to create an HTTP POST message
void setUpPostMsg(char msg[MAX], char string[MAX]){
	//Set up headder
	char head[MAX];
	sprintf(head,"%s%s%d\r\n%s", postFirstLine,
								contentLine,
	 							(int) strlen(string),
	 							staticGatewayNum);
	//set up body/message
	sprintf(msg,"%s%s\r\n",head,string);
}