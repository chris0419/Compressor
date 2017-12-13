//ADD eader information here
#include "compression.h"

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
	serverAddress.sin_port = htons(500);

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

		status = read(clientSocket, stringBuffer, 255);
		errorChecker(status, "ERROR READING FROM CLIENT");

		status = write(clientSocket, "Welcome!", 9);
		errorChecker(status, "ERROR WRITING TO CLIENT");

		printf("%s\n", stringBuffer);
	}



	return 0;
}


