//ADD eader information here
#include "compression.h"


void sendResponse(int socket, char* message)
{
	char header[9];
	char payloadlen[3];
	char statusCode[3];
	char* response;
	int bytesSent;

	sprintf(header, "%x", htonl(MGN));
	sprintf(payloadlen,"%02X", htons(strlen(message)));
	sprintf(statusCode,"%02X", htons(cStat.errorCode));

	//Calculate the size of the header + the message/payload
	bytesSent = strlen(header) + strlen(payloadlen) + strlen(statusCode) + strlen(message) + 1;
	response = malloc(bytesSent);

	if (response == NULL)
	{
		cStat.errorCode = LOW_MEM_ERR;
		return;
	}

	//put the header, payload length, status, plus payload into the full message
	response = strcat(response, header);
	response = strcat(response, payloadlen);
	response = strcat(response, statusCode);
	response = strcat(response, message);

	send(socket, response, strlen(response), 0);

	//Increse the counts send
	cStat.bytesSent += bytesSent;
	free (response);
}

void getStats(int socket)
{
	char bytesSent[33];
	char bytesRec[33];
	char ratio[2];
	char message[66];

	sprintf(bytesSent,"%08X", ntohl(cStat.bytesSent));
	sprintf(bytesRec, "%08X", ntohl(cStat.bytesRead));

	if (cStat.bytesSent <= 0)
	{
		sprintf(ratio, "%01X", ntohs(1));
	}
	else
	{
		sprintf(ratio, "%01X", ntohs(cStat.bytesSent/cStat.bytesRead * 100));
	}


	strcat(message, bytesSent);
	strcat(message, bytesRec);
	strcat(message, ratio);

	sendResponse(socket, message);
}

void reset(int socket){
	initializeStatus();
	sendResponse(socket, "\0");
}

void parseReadData(int socket)
{
	int status;
	uint32_t magicNumber;
	uint16_t payloadlen;
	uint16_t RCval;
	char stringBuffer[64];
	char *end;

	//Get the magic number header number
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, 10);
	if (status < 10)				//The header read less than 10 bytes. The entire packet was too short.
	{
		cStat.errorCode = HEADER_PARSING_ERR;
	}
	cStat.bytesSent += status;
	magicNumber = (uint32_t)strtol(stringBuffer, &end, 16);
	magicNumber = (uint32_t)ntohl(magicNumber);
	if ((*end != '\0') || (magicNumber != MGN))
	{
		cStat.errorCode = HEADER_MAGIC_ERR;
	}


	//Get the payload length
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, 4);
	if (status < 2)
	{
		cStat.errorCode = HEADER_PARSING_ERR;
	}
	cStat.bytesSent += status;
	payloadlen = (uint16_t) strtol(stringBuffer, &end, 16);
	payloadlen = ntohs((uint16_t)payloadlen);
	if (*end != '\0')
	{
		cStat.errorCode = HEADER_SIZE_ERR;
	}

	else if (payloadlen >= MAX_PAYLOAD_LEN)
	{
		cStat.errorCode = MSG_SIZE_ERR;
	}


	//Get the RC code
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, 4);
	if (status < 2)
	{
		cStat.errorCode = HEADER_PARSING_ERR;
	}
	cStat.bytesSent += status;
	RCval = (uint16_t) strtol(stringBuffer, &end, 16);
	RCval = ntohs(RCval);
	if (*end != '\0')
	{
		cStat.errorCode = HEADER_RC_ERR;
	}

	switch (RCval)
	{
		case 1: //todo add ping messages
			break;
		case 2:
			getStats(socket);
			break;
		case 3:
			reset(socket);
			break;
		case 4:
			//todo compression algorithmn
			break;
		default:
			cStat.errorCode = HEADER_RC_ERR;
	}

	return;

}


void errorChecker(int status, char * msg)
{
	if (status < 0)
	{
		perror(msg);
		exit(1);
	}

}
//initialize the inital members of status structure
void initializeStatus()
{
	cStat.errorCode = OK_STATUS;
	cStat.bytesRead = 0;
	cStat.bytesSent = 0;
}

int main() {
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	int serverSocket;
	int clientSocket;
	socklen_t clientSocketSize;
	int status;

	initializeStatus();

	//Get the TCP socket  with the IPv4 Address family
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	errorChecker (serverSocket, "ERROR ACQUIRING SOCKET");

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(PORTNUM);

	status = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	errorChecker(status, "ERROR BINDING SOCKET WITH ADDRESS");

	//wait until you get a response
	status = listen(serverSocket, 1);
	errorChecker(status, "ERROR LISTENING FOR SOCKET");


	while(1)
	{
		clientSocketSize = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientSocketSize);
		errorChecker(clientSocket, "ERROR ACCEPTING CLIENT SOCKET");
		parseReadData(clientSocket);
	}

	return 0;
}


