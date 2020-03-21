/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: messages.h
 *
 *  Description: Declerations and function prototypes for messages structs.
 *  This module will be used to generate messages that will be sent between
 *  peer node and index server using TCP module.  
 *  
 *==========================================================================*/
#include <string>

#ifndef MESSAGES_H_
#define MESSAGES_H_

/*===========================================================================
 * Message structs declarations
 *==========================================================================*/
typedef enum {
    REGISTER,
    REGISTER_CONFIRM,
    REGISTER_ACK,
    REQ_IDX,
    FILE_FOUND,
    ERR_FILE_NOT_FOUND,
    REQ_PEER,
    DATA,
    REPORT
} message_type;

/* Register message */
typedef struct __attribute__((__packed__)) register_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
    char seeder_ip[16];
    char seeder_portno[6];
    char file_hash[64];
} RegisterMsg;

/* Register confirm message */
typedef struct __attribute__((__packed__)) register_confirm_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
} RegisterConfirmMsg;

/* Register ACK message */
typedef struct __attribute__((__packed__)) register_ack_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
    char file_hash[64];
} RegisterAckMsg;

/* Request file from index server message */
typedef struct __attribute__((__packed__)) req_idx_msg {
    unsigned short type;
    char file_name[20];
    char leecher_ip[16];
    char leecher_portno[6];
} ReqIdxMsg;

/* Index server response - found file message */
typedef struct __attribute__((__packed__)) file_found_msg {
    unsigned short type;
    char seeder_ip[16];
    char seeder_portno[6];
} FileFoundMsg;

/* Index server response - error file not found message */
typedef struct __attribute__((__packed__)) err_file_not_found_msg {
    unsigned short type;
} ErrFileNotFoundMsg;

/* Request file from peer server message */
typedef struct __attribute__((__packed__)) req_peer_msg {
    unsigned short type;
    char file_name[20];
} ReqPeerMsg;

/* Report invalid IP to index server message */
typedef struct __attribute__((__packed__)) report_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
    char leecher_ip[16];
    char leecher_portno[6];
    char seeder_ip[16];
    char seeder_portno[6];
} ReportMsg;

/*===========================================================================
 * Message structs function prototypes
 *==========================================================================*/
RegisterMsg *create_register_msg(message_type, int, std::string, 
                        std::string, std::string, std::string);

RegisterAckMsg *create_register_ack_msg(message_type, int, std::string, 
                        std::string);

ReqIdxMsg *create_reqidx_msg(message_type, std::string, std::string, 
                        std::string);

FileFoundMsg *create_file_found_msg(message_type, std::string, std::string);

ErrFileNotFoundMsg *create_err_file_not_found_msg(message_type);

ReqPeerMsg *create_reqpeer_msg(message_type, std::string);

ReportMsg *create_report_msg(message_type, int, std::string, 
                        std::string, std::string, std::string, std::string);

#endif
