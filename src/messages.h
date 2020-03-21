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
#include <cstring>
#include <string>
#include <assert.h>
#include <arpa/inet.h>

/*===== Uncomment for debugging =====*/
#define DEBUG_MESSAGE
/*===================================*/

#ifdef DEBUG_MESSAGE
#include <iostream>
#endif

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
    unsigned short seeder_portno;
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
    unsigned short leecher_portno;
} ReqIdxMsg;

/* Index server response - found file message */
typedef struct __attribute__((__packed__)) file_found_msg {
    unsigned short type;
    char seeder_ip[16];
    unsigned short seeder_portno;
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
    unsigned short leecher_portno;
    char seeder_ip[16];
    unsigned short seeder_portno;
} ReportMsg;

/* Data message struct */
typedef struct data_msg {
    unsigned short type;
    unsigned int file_size;
    char *data;
} DataMsg;
/*===========================================================================
 * Message structs function prototypes
 *==========================================================================*/

/* Creating messages */
RegisterMsg *create_register_msg(int, std::string, std::string, unsigned short,
                                                std::string);
RegisterConfirmMsg *create_reg_confirm_msg(int, std::string);
RegisterAckMsg *create_register_ack_msg(int, std::string, std::string);
ReqIdxMsg *create_reqidx_msg(std::string, std::string, unsigned short);
FileFoundMsg *create_file_found_msg(std::string, unsigned short);
ErrFileNotFoundMsg *create_err_file_not_found_msg();
ReqPeerMsg *create_reqpeer_msg(std::string);

ReportMsg *create_report_msg(int, std::string, std::string, unsigned short, 
                                                std::string, unsigned short);
char *create_data_msg(int, char *);

/* Parsing messages */
void parse_register_msg(char [], RegisterMsg &);
void parse_register_confirm_msg(char [], RegisterConfirmMsg &);
void parse_register_ack_msg(char [], RegisterAckMsg &);
void parse_reqidx_msg(char [], ReqIdxMsg &);
void parse_file_found_msg(char [], FileFoundMsg &);
void parse_err_file_not_found_msg(char [], ErrFileNotFoundMsg &);
void parse_reqpeer_msg(char [], ReqPeerMsg &);
void parse_report_msg(char [], ReportMsg &);
void parse_data_msg(char [], DataMsg &);

#endif
