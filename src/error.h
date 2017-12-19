/*
 * Created By: Christopher Rivera Reyes
 *
 * ERROR header full of error codes.
 */

#ifndef ERROR__H
#define ERROR__H

#define OK_STATUS 				0
#define UNKNOWN_STATUS_ERR		1
#define MSG_SIZE_ERR 			2	//Message over 32KB or under 4KB
#define UNSUPPORT_TYPE_ERR		3	//Unsupported RC type

// RERSERVE_STATUS 4-32

#define HEADER_MAGIC_ERR 		33	//The magic number didn't match
#define HEADER_SIZE_ERR			34	//Payload Length issue
#define HEADER_RC_ERR			35	//Problem with the RC
#define HEADER_PARSING_ERR		36	//Parsing through the header problem

#define PAYLOAD_INT_ERR			37	//payload contains int
#define PAYLOAD_CAP_ERR 		38	//payload contains capital letter
#define PAYLOAD_INVALID_ERR		39	//payload contains non letter char

#define LOW_MEM_ERR				40
#endif /* ERROR__H */
