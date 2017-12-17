#ifndef COMPRESSION__H
#define COMPRESSION__H

//HEADERS
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

//PROTOTYPEs
void errorChecker(int, char *);
void parseReadData(int);
void initializeStatus();

//DEFINES
#define PORTNUM 4000		  //The port number in which the two sockets with be connected to
#define MGN 0x53545259        //The magic number in front of the header
#define MAX_PAYLOAD_LEN 32768 //32KB for maximum payload length
#define MIN_PAYLOAD_LEN 4096  //4KB for the minimum payload length

struct Status{
	int errorCode;
	uint32_t bytesRead;
	uint32_t bytesSent;

} cStat;



#endif //COMPRESSION__H
