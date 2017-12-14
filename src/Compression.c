//ADD eader information here
#include "compression.h"



void parseReadData(int socket)
{
	int status;
	int magicNumber;
	int payloadlen;
	int RCval;
	char stringBuffer [256];
	char *end;

	status = read(socket, stringBuffer, 10);
	errorChecker(status, "ERROR READING FROM MAGIC HEADER");
	magicNumber = (int)strtol(stringBuffer, &end, 0);

	if (*end != '\0')
	{
		//RESET this to be an soft error not a hard error.
		errorChecker(-1, "HEADER doesn't start with 0x or is not a number");
	}

	else if (magicNumber != MGN)
	{
		errorChecker(-1, "HEADER DOESNT CONTAIN 0x53545259");
	}

	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, 2);
	errorChecker(status, "ERROR READING PAYLOAD LENGTH");
	payloadlen = (int) strtol(stringBuffer, &end, 0);

	if (*end != '\0')
	{
		errorChecker(-1, "Payload Length was not a number or ");
	}

	else if (payloadlen <= MAX_PAYLOAD_LEN)
	{
		errorChecker(-1, "PayLoad Length was over 32KB");
	}

	status = read(socket, stringBuffer, 2);
	errorChecker(status, "ERROR READING RC OF HEADER");
	RCval = (int) strtol(stringBuffer, &end, 0);

	if (*end != '\0')
	{
		errorChecker(-1, "RC number was not a number or ");
	}

	switch (RCval)
	{
		case 1: //todo add  ping message
			break;
		case 2:
			//todo add statistics
			break;
		case 3:
			//todo reset stats
			break;
		case 4:
			//todo compression algorithmn
			break;
		default:
			errorChecker(-1, "ERROR GETTING REQUEST VALUE");
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

int main() {
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	int serverSocket;
	int clientSocket;
	socklen_t clientSocketSize;
	int status;
	char stringBuffer[256];

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


