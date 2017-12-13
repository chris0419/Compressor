//ADD eader information here


#include "compression.h"

void errorChecking(int status, char * msg)
{
	if (status < 0)
	{
		cerr <<  msg << strerror(status) << endl;
		exit(0);
	}

}

int main() {
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int serverSocket;
	int status;

	//Get the TCP socket  with the IPv4 Address family
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket < 0)
	{
		cerr <<  "ERROR ACQUIRING SOCKET. ERROR #" << strerror(serverSocket) << endl;
	}


	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(500);

	status = bind(serverSocket, (struct sockaddr *) &serverSocket, sizeof(serverSocket));



	//wait until you get a response
	status = listen(serverSocket, 1);
	if (status < 0)
	{
		cerr <<  "ERROR LISTENING FOR SOCKET. ERROR #" << strerror(status) << endl;
	}




	return 0;
}
IT_SUCCESS;
}
