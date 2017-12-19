# Compressor

This is a compression service which takes in data through an TCP socket. It will compress data by counting the number of ASCII letters in the message and returning it to the user.

##PREREQUISITE
Written in C and tested on Ubuntu4.4.0.
## HEADER MAGIC NUMBER
In order for the service to work it will need a proper header. It's expecing the magic number 0x53545259. After aquiring the first eight bytes, it will check if the number acquired is equal to the magic nuber.If the first eight bytes do not match the magic number, it will return an error code.
## PAYLOAD LENGTH
Following the magic number is the payload length or message length. It tells the service how big the message is. The service will not read more than the payload length.
## RC/STATUS
Following the payload length there is the request code or status code in response. 
### PING
If RC equal to one, the service will return a magic value header with a payload length of 0, and the status of the system. If there was an error code that occur then it will return it. 

The service tracks its status by having a struct variable called cStat.
It tracks the bytes read/sent and the error code.
```
struct Status{
	int errorCode;
	uint32_t bytesRead;
	uint32_t bytesSent;
} cStat;
```
If an error occurred it will be set until a reset request was sent.
### GETSTAT
IF RC equal two, service will return a valid header along with the size of bytes sent, bytes recieved, and the ratio between the two. It uses the Status struct created 
### RESET
If RC equal to three, the service will reset the stats structure. This means the error code will be cleared. This is the RC that is needed to remove an error ode from the system.
### COMPRESS
If RC equal to four, the compression algorithm read a payload length worth of bytes from the socket. It takes the first character and counts all following character that equals itself. If the following counter is different, it saves the count and its respective  character into a linked list liked structured called Entry. 

```
struct Entry{
    char character;
    int count;
    struct Entry* next;
}; 
```

Only when it reaches a new character or a null character it will add it to the Entry list. After reading through the payload/message, the algorithm will go through the list of entires and create a message to sent back to the client.

## NETWORK BYTE ORDER
All header fields: Magic Number, Payload Length, RC/Status are in network byte order or big-endian seperately. For every seperate field, the client must convert from network byte order to the endianess of their machine. The payload/message is not needed for conversion.
## ERROR HANDLING
There are multiple error codes that can be returned. One can read this in the error.h header file.

```
#define OK_STATUS 				0	//Everything is Okay
#define UNKNOWN_STATUS_ERR		1	//Unknown Error
#define MSG_SIZE_ERR 			2	//Message over 32KB or under 4KB
#define UNSUPPORT_TYPE_ERR		3	//Unsupported RC type



#define HEADER_MAGIC_ERR 		33	//The magic number didn't match
#define HEADER_SIZE_ERR			34	//Payload Length issue
#define HEADER_RC_ERR			35	//Problem with the RC Header
#define HEADER_PARSING_ERR		36	//Parsing through the header problem 
									//such as getting a character

#define PAYLOAD_INT_ERR			37	//payload contains int
#define PAYLOAD_CAP_ERR 		38	//payload contains capital letter
#define PAYLOAD_INVALID_ERR		39	//payload contains non letter char

#define LOW_MEM_ERR				40	//allocating memory failed.
```


