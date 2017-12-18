//ADD eader information here
#include "compression.h"

char* compressionResponse(char *response, struct Entry* head)
{
	struct Entry* node;
	char chalkboard[3];

	node = head;
	while(node != NULL)
	{
		if (node->count == 1)
		{
			sprintf(chalkboard, "%c", node->character);
		}
		else if (node->count == 2)
		{
			sprintf(chalkboard, "%c%c", node->character, node->character);
		}
		else
		{
			sprintf(chalkboard, "%d%c",node->count, node->character);

		}
		node = node->next;
		strcat(response, chalkboard);
	}

	return response;
}

int encoding(struct Entry *head, char* message)
{
	struct Entry* temp;
	struct Entry* node;
	char character;
	char *letter;
	int counter;


	node = head;
	character = *message;
	counter = 0;


	for (letter = message; *letter != '\0'; letter++)
	{
		if (character <= '0' && character >= '9')
		{
			return PAYLOAD_INT_ERR;
		}

		else if (character >= 'A' && character <= 'Z')
		{
			return PAYLOAD_CAP_ERR;
		}

		else if (character < 'a' && character > 'z')
		{
			return PAYLOAD_INVALID_ERR;
		}
		else if ((character != *letter) || (*(letter + 1) == '\0'))
		{
			if ((*(letter + 1) == '\0') && character == *letter) //handles an edge case
				counter++;

			//add it to the linked list and increase the size
			temp = (struct Entry *) malloc(sizeof(struct Entry));
			temp->character = character;
			temp->count = counter;
			temp->next = NULL;
			head->count += 1;
			node->next = temp;
			node = temp;

			//reset Counter and the Character in question
			counter = 1;
			character = *letter;
		}
		else
		{
			counter++;
		}
	}

	return OK_STATUS;

}
void compression(int socket, uint16_t payloadLen)
{
	struct Entry head;
	char *message;
	char *response;
	int status;

	message = (char *)malloc(payloadLen + 1);
	if(message == NULL)
	{
		//todo must send do someting with this error
		cStat.errorCode = LOW_MEM_ERR;
	}

	//read the data
	status = read(socket, message, payloadLen + 1);
	if (status < payloadLen)				//The header read less than 10 bytes. The entire packet was too short.
	{
		cStat.errorCode = HEADER_SIZE_ERR;
	}

	//parse the data
	head.character = '\0';
	head.count = 0;
	head.next = NULL;
	cStat.errorCode = encoding(&head, message);

	//Create response message
	if (cStat.errorCode == PAYLOAD_INT_ERR)
	{
		response = (char*) malloc();

	}
	else if (cStat.errorCode == PAYLOAD_CAP_ERR)
	{

	}
	else if (cStat.errorCode == PAYLOAD_INVALID_ERR)
	{

	}
	else
	{
		response = (char*) malloc((head.count * 2) + 1); //allocate 2 bytes for every entry
		response = compressionResponse(response, head.next);
		sendResponse(socket, response);

	}

	free(response);
	free(message);
}

void sendResponse(int socket, char* message)
{
	char header[9];
	char payloadlen[3];
	char statusCode[3];
	char* response;
	int bytesSent;

	sprintf(header, "%x", htonl(MGN));
	sprintf(payloadlen,"%04X", htons(strlen(message)));
	sprintf(statusCode,"%04X", htons(cStat.errorCode));

	//Calculate the size of the header + the message/payload
	bytesSent = strlen(header) + strlen(payloadlen) + strlen(statusCode) + strlen(message) + 1;
	response = (char *)malloc(bytesSent);

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
		sprintf(ratio, "%02X", ntohs(1));
	}
	else
	{
		sprintf(ratio, "%02X", ntohs(cStat.bytesSent/cStat.bytesRead * 100));
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


//todo, when there is a problem. Give an appropiate response.
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
	status = read(socket, stringBuffer, MAX_PAYSTAT_BYTE);
	if (status < MAX_PAYSTAT_BYTE)
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
	status = read(socket, stringBuffer, MAX_PAYSTAT_BYTE);
	if (status < MAX_PAYSTAT_BYTE)
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
			compression(socket, payloadlen);
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


