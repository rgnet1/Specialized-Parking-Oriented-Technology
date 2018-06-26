
#define MAX 10000
#define tMAX 10
#define GETTIMEOUT 120 //120 is 10 min
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include "jsonParse.c"


//          struct tdata
// Data is passed to a particular thread
// via the tdata struct
typedef struct{
	int socketFD;
	char *buffer;
	int index;
}tdata;

//      createConn()
// 
// @param: port
//
// Creates a listening connection at the
// specified port
void createConn(int port);


//      checkInNOw()
//
// check all sensors for the 15 min check in
void checkInNow();

//      connToAppEngine()
//
// @return: port
//
// Creates a connecction to the app engine
// and returns the port used to make this
// connection.
int connToAppEngine();


//       getData()
//
// @param: tData
//
// This function is called whenever a new sensor
// has data to send. This function gets the data
// from the sensors and decides what to do with it
void* getData(void* tData);


//        talkToAppEngine()
//
// @param: threadData, socketAE, array, newbuff
//
// formats and sends an HTTP GET request to send
// and recieve data from the Google Cloud App Engine.
void talkToAppEngine(tdata *threadData, int socketAE, char *array[2], char newbuff[MAX]);

//        handleAppEngineTimeout()
//
// @param: tdata, socketAE, array[2]
//
// If the App Engine doesn't respond within a reasonable
// time, this function is called.
void handleAppEngineTimeOut(tdata *threadData, int socketAE, char *array[2]);


//            waitForAppEngine()
//
// @param: threadData, array
//
// This function takes in the thread data and 
// array containing spot information. It waits
// for the App Engine to respond with this threads'
// sensor status
void waitForAppEngine(tdata *threadData, char *array[2]);


//        setUpPostMsg()
//
// @param: msg, string
//
// Takes a buffer for the final msg ressult and 
// a string body to create an HTTP POST message
void  setUpPostMsg(char msg[MAX], char string[MAX]);