/*
 * error.h
 *
 *  Created on: Dec 13, 2017
 *      Author: chris
 */

#ifndef ERROR__H
#define ERROR__H

#define OK_STATUS 				0
#define UNKNOWN_STATUS_ERR		1
#define MSG_SIZE_ERR 			2	//Message over 32KB or under 4KB
#define UNSUPPORT_TYPE_ERR		3

// RERSERVE_STATUS 4-32

#define HEADER_MAGIC_ERR 		33	//The magic number didn't match
#define HEADER_SIZE_ERR			34	//Payload Length issue
#define HEADER_RC_ERR			35	//Problem with the RC
#define HEADER_PARSING_ERR		36	//Parsing through the header problem

#define PAYLOAD_INT_ERR			37
#define PAYLOAD_CAP_ERR 		38
#define PAYLOAD_INVALID_ERR		39

#define LOW_MEM_ERR				40
#endif /* ERROR__H */
