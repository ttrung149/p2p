/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: messages.cpp
 *
 *  Description: Function declarations for messages structs.
 *  This module will be used to generate messages that will be sent between
 *  peer node and index server using TCP module.
 *  
 *==========================================================================*/
#include "messages.h"

/*============================ Create messages =============================*/
/**
 * Create register message 
 * @param 
 */
RegisterMsg *create_register_msg(message_type type, int file_sz, 
    std::string file_name, std::string ip, unsigned short port, std::string hash)
{
    RegisterMsg *msg = (RegisterMsg *)malloc(sizeof(RegisterMsg));
    assert(msg);

    msg->type = (unsigned short) htons(type);
    msg->file_size = htonl(file_sz);

    bzero(msg->file_name, 20);
    strncpy(msg->file_name, file_name.data(), 20);  
    bzero(msg->seeder_ip, 16);
    strncpy(msg->seeder_ip, ip.data(), 16);
    msg->seeder_portno = (unsigned short) htons(port);
    bzero(msg->file_hash, 64);
    strncpy(msg->file_hash, hash.data(), 64);

    return msg;
}

/*============================= Parse messages =============================*/
/**
 * Parse register message 
 * @param 
 */

void parse_register_msg(char buffer[], RegisterMsg &msg) 
{
    char type_buffer[2];
    memcpy(type_buffer, buffer, 2);
    msg.type = (message_type) ntohs(*(unsigned short *)(type_buffer));


    char file_sz_buffer[4];
    memcpy(file_sz_buffer, buffer + 2, 4);
    msg.file_size = ntohl(*(unsigned int *)(file_sz_buffer));

    strncpy(msg.file_name, buffer + 6, 20);
    strncpy(msg.seeder_ip, buffer + 26, 16);

    char portno_buffer[2];
    memcpy(portno_buffer, buffer + 42, 2);
    msg.seeder_portno = ntohs(*(unsigned short *)(portno_buffer));

    strncpy(msg.file_hash, buffer + 44, 64);

    #ifdef DEBUG_MESSAGE
    std::cout << "\n======================================"
              << "\n\tParsing REGISTER_MSG"
              << "\n======================================"
              << "\nType: "         << msg.type
              << "\nFile size: "    << msg.file_size
              << "\nFile name "     << msg.file_name
              << "\nSeeder IP: "    << msg.seeder_ip
              << "\nSeeder port: "  << msg.seeder_portno
              << "\nFile hash: "    << msg.file_hash << std::endl;
    #endif
}