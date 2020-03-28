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
//#define DEBUG_MESSAGE
/*===================================*/

#ifdef DEBUG_MESSAGE
#include <iostream>
#endif

#ifndef MESSAGES_H_
#define MESSAGES_H_

/*===========================================================================
 * Message structs declarations
 *==========================================================================*/
const int DATA_MSG_BUF_SIZE = 8000;

typedef enum {
    REGISTER,
    REGISTER_CONFIRM,
    REGISTER_ACK,
    FILE_FOUND,
    ERR_FILE_NOT_FOUND,
    REQ_FILE,
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
    char index_ip[16];
    unsigned short index_portno;
} RegisterConfirmMsg;

/* Register ACK message */
typedef struct __attribute__((__packed__)) register_ack_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
    char seeder_ip[16];
    unsigned short seeder_portno;
    char file_hash[64];
} RegisterAckMsg;

/* Index server response - found file message */
typedef struct __attribute__((__packed__)) file_found_msg {
    unsigned short type;
    char file_name[20];
    char seeder_ip[16];
    unsigned short seeder_portno;
    char file_hash[64];
} FileFoundMsg;

/* Index server response - error file not found message */
typedef struct __attribute__((__packed__)) err_file_not_found_msg {
    unsigned short type;
} ErrFileNotFoundMsg;

/* Request file from peer/index server message */
typedef struct __attribute__((__packed__)) req_file_msg {
    unsigned short type;
    char file_name[20];
    char leecher_ip[16];
    unsigned short leecher_portno;
} ReqFileMsg;

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
typedef struct __attribute__((__packed__)) data_msg {
    unsigned short type;
    unsigned int file_size;
    char file_name[20];
    unsigned int segment_size;
    unsigned int segno;
    char seeder_ip[16];
    unsigned short seeder_portno;
    char data[DATA_MSG_BUF_SIZE];
} DataMsg;

/*===========================================================================
 * Message structs function prototypes
 *==========================================================================*/

/* Creating messages */
RegisterMsg *create_register_msg(int, std::string, std::string, unsigned short,
                                                std::string);
RegisterConfirmMsg *create_reg_confirm_msg(int, std::string, std::string, 
                                                unsigned short);
RegisterAckMsg *create_register_ack_msg(int, std::string, std::string, 
                                                unsigned short, std::string);
FileFoundMsg *create_file_found_msg(std::string, std::string, unsigned short,
                                                std::string);
ErrFileNotFoundMsg *create_err_file_not_found_msg();
ReqFileMsg *create_reqfile_msg(std::string, std::string, unsigned short);

ReportMsg *create_report_msg(int, std::string, std::string, unsigned short, 
                                                std::string, unsigned short);
DataMsg *create_data_msg(int, int, int, std::string, std::string, 
                                                unsigned short, char *);

/* Parsing messages */
void parse_register_msg(char [], RegisterMsg &);
void parse_register_confirm_msg(char [], RegisterConfirmMsg &);
void parse_register_ack_msg(char [], RegisterAckMsg &);
void parse_file_found_msg(char [], FileFoundMsg &);
void parse_err_file_not_found_msg(char [], ErrFileNotFoundMsg &);
void parse_reqfile_msg(char [], ReqFileMsg &);
void parse_report_msg(char [], ReportMsg &);
void parse_data_msg(char [], DataMsg &);

#endif
