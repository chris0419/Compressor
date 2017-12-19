/**
 * Created By:Christopher Rivera Reyes
 *
 * A compression service which recieves data over a
 * TCP socket.
 */
#include "compression.h"


/**
 * Returns an error string back to the caller
 */
const char* getErrorStat(int errono)
{
	switch (errono)
	{
		case OK_STATUS:
			return  "\0";
		case UNKNOWN_STATUS_ERR:
			return "UNKNOWN STATUS OCCURED";
		case MSG_SIZE_ERR:
			return "MESSAGE TOO LARGE OR TOO SMALL";
		case UNSUPPORT_TYPE_ERR:
			return "UNSUPORTED REQUEST TYPE";
		case HEADER_MAGIC_ERR:
			return "HEADER DOESN'T CONTAIN CORRECT 0x53545259";
		case HEADER_SIZE_ERR:
			return "HEADER SIZE ERROR. TOO BIG OR TOO SMALL.";
		case HEADER_RC_ERR:
			return "FAILED TO READ REQUEST TYPE";
		case HEADER_PARSING_ERR:
			return "FAILED TO READ HEADER";
		case PAYLOAD_INT_ERR:
			return "MESSAGE CONTAINED INTEGER";
		case PAYLOAD_CAP_ERR:
			return "MESSAGE CONTAINS A CAPITAL LETTER";
		case PAYLOAD_INVALID_ERR:
			return "MESSAGE CONTAINS CHARACTERS OTHER THAN ALPHABET";
		case LOW_MEM_ERR:
			return "SYSTEM CONTAINS LOW MEMORY";
		default:
			return "INVALID ERROR";
	}
}

/*
 * Sends the status of the system
 */
void sendStatus(int socket)
{
	char* message = (char *)getErrorStat(cStat.errorCode);
	sendResponse(socket, message);
}

/**
 * Iterates through a linkedlist structure called Entry
 * and place the contents into a string to return to the client
 */
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

/**
 * Deallocates Memory stored in the LinkedList struct
 */
void destroyLL(struct Entry *head)
{
	if (head->next != NULL)
	{
		destroyLL(head->next);
	}
	free(head);
}

/*
 * Encoding algorithm which counts the number of sequential characters
 * It places the character and its associated number to to a linkedlist
 * liked structure called Entry.
 */
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

	//For every letter in the message, compare to the previous letter
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

/*
 * Encodes the payload message by creating a linkedlist type struct called Entry.
 * This returns a head node of all the list of all characters with the counter
 * associated with that character.
 */
void compression(int socket, uint16_t payloadLen)
{
	struct Entry head;
	char *message;
	char *response;
	int status;
	int bytesLeft;

	//Check if the payloadlen
	if (payloadLen < MIN_PAYLOAD_LEN)
	{
		cStat.errorCode = MSG_SIZE_ERR;
		return sendStatus(socket);
	}

	message = (char *)malloc(payloadLen + 1);
	if(message == NULL)
	{
		cStat.errorCode = LOW_MEM_ERR;
		return sendStatus(socket);
	}

	//read the data, and makes sure the payload is within range
	bytesLeft = payloadLen;
	do
	{
		status = read(socket, message, bytesLeft);
		bytesLeft -= status;
	} while(bytesLeft);
	cStat.bytesRead += payloadLen;

	//parse the data
	head.character = '\0';
	head.count = 0;
	head.next = NULL;
	cStat.errorCode = encoding(&head, message);

	//Create response message
	if (cStat.errorCode != OK_STATUS)
	{
		sendStatus(socket);

	}
	else
	{
		//allocate enough memory for the message then slim it down to save mem
		response = (char*) malloc(payloadLen);
		if (response ==  NULL)
		{
			cStat.errorCode = LOW_MEM_ERR;
			return sendStatus(socket);
		}
		response = compressionResponse(response, head.next);
		response = (char*) realloc(response, strlen(response));
		sendResponse(socket, response);
	}

	//free all the dynamic memory
	destroyLL(head.next);
	free(response);
	free(message);
}

/**
 * Sends a message back to the client.
 */
void sendResponse(int socket, char* message)
{
	char header[9];
	char payloadlen[5];
	char statusCode[5];
	char* response;
	int bytesSent;

	sprintf(header, "%x", htonl(MGN));
	sprintf(payloadlen,"%04X", htons(strlen(message)));
	sprintf(statusCode,"%04X", htons(cStat.errorCode));

	//Calculate the size of the header + the payload
	bytesSent = sizeof(header) + sizeof(payloadlen) + sizeof(statusCode) + strlen(message) + 1;
	response = (char *)malloc(bytesSent);
	if (response == NULL)
	{
		cStat.errorCode = LOW_MEM_ERR;
		return sendStatus(socket);
	}

	//put the header, payload length, status, plus payload into the full message
	memset(response, '\0', bytesSent);
	response = strcat(response, header);
	response = strcat(response, payloadlen);
	response = strcat(response, statusCode);
	response = strcat(response, message);

	send(socket, response, bytesSent, 0);

	//Increse the counter of bytes sent and free memory
	cStat.bytesSent += bytesSent;
	free (response);
}

/**
 * Gets the bytes sent, received, and the ratio between the two and
 * sends it to the client.
 */
void getStats(int socket)
{
	char bytesSent[9];
	char bytesRec[9];
	char ratio[3];
	char message[18];

	//clear the array with null chars
	memset(bytesSent, '\0', sizeof(bytesSent));
	memset(bytesRec,'\0', sizeof(bytesRec));
	memset(ratio, '\0', sizeof(ratio));
	memset(message, '\0', sizeof(message));

	sprintf(bytesSent,"%08X", ntohl(cStat.bytesSent));
	sprintf(bytesRec, "%08X", ntohl(cStat.bytesRead));

	//If no bytes were sent then the ratio is 0
	if (cStat.bytesSent <= 0)
	{
		sprintf(ratio, "%02X", 0);
	}
	else
	{
		sprintf(ratio, "%02X", (int)((float)cStat.bytesSent/cStat.bytesRead *100));
	}

	strcat(message, bytesSent);
	strcat(message, bytesRec);
	strcat(message, ratio);

	sendResponse(socket, message);
}

/**
 * Resets the status struct and sends appropriate response
 */
void reset(int socket)
{
	initializeStatus();
	sendResponse(socket, "\0");
}

/**
 * This parses the header coming from a socket. It's expecting the magic number
 * followed by the payload length and RC code. If an error occurs parsing the
 * header then a right error response will be sent.
 */
void parseReadData(int socket, int serverSocket)
{
	int status;
	uint32_t magicNumber;
	uint16_t payloadlen;
	uint16_t RCval;
	char stringBuffer[64];
	char *end;

	//Get the magic number header number and confirming that it's the magic number
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, 8);
	if (status < 8)		//The header read less than 8 bytes. The entire packet was too short.
	{
		cStat.errorCode = HEADER_PARSING_ERR;
		return sendStatus(socket);
	}
	cStat.bytesRead += status;
	magicNumber = (uint32_t)strtol(stringBuffer, &end, 16);
	magicNumber = (uint32_t)ntohl(magicNumber);
	if ((*end != '\0') || (magicNumber != MGN))
	{
		cStat.errorCode = HEADER_MAGIC_ERR;
		return sendStatus(socket);
	}

	//Get the payload length
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, MAX_PAYSTAT_BYTE);
	if (status < MAX_PAYSTAT_BYTE)
	{
		cStat.errorCode = HEADER_PARSING_ERR;
		return sendStatus(socket);
	}
	cStat.bytesRead += status;
	payloadlen = (uint16_t) strtol(stringBuffer, &end, 16);
	payloadlen = ntohs((uint16_t)payloadlen);
	if (*end != '\0')
	{
		cStat.errorCode = HEADER_SIZE_ERR;
		return sendStatus(socket);
	}

	else if (payloadlen >= MAX_PAYLOAD_LEN)
	{
		cStat.errorCode = MSG_SIZE_ERR;
		return sendStatus(socket);
	}


	//Get the RC code
	memset(stringBuffer, 0, sizeof(stringBuffer));
	status = read(socket, stringBuffer, MAX_PAYSTAT_BYTE);
	if (status < MAX_PAYSTAT_BYTE)
	{
		cStat.errorCode = HEADER_PARSING_ERR;
		return sendStatus(socket);
	}
	cStat.bytesRead += status;
	RCval = (uint16_t) strtol(stringBuffer, &end, 16);
	RCval = ntohs(RCval);
	if (*end != '\0')
	{
		cStat.errorCode = HEADER_RC_ERR;
		return sendStatus(socket);
	}

	switch (RCval)
	{
		case 1:
			sendStatus(socket);
			break;
		case 2:
			getStats(socket);
			break;
		case 3:
			reset(socket);
			break;
		case 4:
			compression(socket, payloadlen);
			break;
		case 5:
			printf("Closing Service");
			close(serverSocket);
			exit(EXIT_SUCCESS);
		default:
			cStat.errorCode = HEADER_RC_ERR;
			sendStatus(socket);
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
/**
 * initialize the inital members of status structure
 */
void initializeStatus()
{
	cStat.errorCode = OK_STATUS;
	cStat.bytesRead = 0;
	cStat.bytesSent = 0;
}

/**
 * Setups the socket necessary for data transfer, and waits for a response
 */
int main() {
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	int serverSocket;
	int clientSocket;
	socklen_t clientSocketSize;
	int status;

	initializeStatus();
	printf("Service Initialized.\n");

	//Get the TCP socket  with the IPv4 Address family
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	errorChecker (serverSocket, "ERROR ACQUIRING SOCKET");

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(PORTNUM);
	printf("Listening on PORTNUM %d.\n", PORTNUM);

	status = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	errorChecker(status, "ERROR BINDING SOCKET WITH ADDRESS");

	//wait until you get a response
	status = listen(serverSocket, 10);
	errorChecker(status, "ERROR LISTENING FOR SOCKET");

	while(1)
	{
		clientSocketSize = sizeof(clientAddress);
		printf("Waiting for client...\n");
		clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientSocketSize);
		errorChecker(clientSocket, "ERROR ACCEPTING CLIENT SOCKET");
		parseReadData(clientSocket, serverSocket);
		printf("Data sent to the client");
		close(clientSocket);
	}
	return 0;
}


