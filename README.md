# Compressor

This is a compression service which takes in data through an TCP socket. It will compress data by counting the number of ASCII letters in the message and returning it to the user.

## PREREQUISITE
Written in C and tested on Ubuntu 4.4.0  using gcc version 5.4.0.
## HEADER MAGIC NUMBER
In order for the service to work it will need a proper header. It's expecing the magic number 0x53545259. After acquiring the first eight bytes, it will check if the number acquired is equal to the magic number.If the first eight bytes do not match the magic number, it will return an error code.
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
IF RC equal two, service will return a valid header along with the size of bytes sent, bytes received, and the ratio between the two. It uses the Status struct created
### RESET
If RC equal to three, the service will reset the stats structure. This means the error code will be cleared. This is the RC that is needed to remove an error ode from the system.
### COMPRESS
If RC equal to four, the compression algorithm read a payload length worth of bytes from the socket. It takes the first character and counts all following character that equals itself. If the following counter is different, it saves the count and its respective  character into a linked list like structured called Entry.

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
There are multiple error codes that can be returned. One can read this in the error.h header file. If there is an error with acquiring the socket, the server will shutdown. If a socket is not acquired then there is nothing that can be done. Other errors are what I call 'soft errors', are error with the data coming from the socket. It ranges from having a bad header or having a number or capital letter in a compression message. It will return a message with the standard header, along with the error code.

```
#define OK_STATUS		0    //Everything is Okay
#define UNKNOWN_STATUS_ERR      1    //Unknown Error
#define MSG_SIZE_ERR            2    //Message over 32KB or under 4KB
#define UNSUPPORT_TYPE_ERR      3    //Unsupported RC type

#define HEADER_MAGIC_ERR        33    //The magic number didn't match
#define HEADER_SIZE_ERR         34    //Payload Length issue
#define HEADER_RC_ERR           35    //Problem with the RC Header
#define HEADER_PARSING_ERR      36    //Parsing through the header problem
                        		//such as getting a character in the header
                        
#define PAYLOAD_INT_ERR         37    //payload contains int
#define PAYLOAD_CAP_ERR         38    //payload contains capital letter
#define PAYLOAD_INVALID_ERR     39    //payload contains non letter char

#define LOW_MEM_ERR             40    //allocating memory failed.
```
## BUILD
Package include a makefile and a build.sh script to build the executable. It also contains a shell script called run.sh to run the executable.  
## Assumptions
This service is essentially doesn't stop unless the user tells it to. Initially, I had an RC of 5 to tell the service to shutdown. I decided to comment that out due not being sure how automation request work. I also assume that the entire message and header will arrive
## How to Improve
There are a lot of improvements that can be made. For one, the service doesn't need to run in an infinite while loop waiting for data to arrive. I could have multiple clients talking to the service, and thus could probably use fork() to create a new process per each attached client. This can be created in another language like C++ and have a new instance of service per each client is another example of having multiple clients. I've also used a string to read from a socket for consistency. This leads me to use convert string to integer which takes time. If I had more time I would make it more robust. I know there could be bad message that could "work" but not intended to work.

Overall this was a very fun project to work on.


