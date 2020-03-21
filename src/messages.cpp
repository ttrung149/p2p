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
 * @param file_sz size of file being registered
 * @param file_name name of file being registered
 * @param ip seeder IP address
 * @param port seeder IP port
 * @param hash SHA-256 hash of file being registered
 * @returns Pointer to newly allocated RegisterMsg struct
 */
RegisterMsg *create_register_msg(int file_sz, std::string file_name,
                        std::string ip, unsigned short port, std::string hash)
{
    RegisterMsg *msg = (RegisterMsg *)malloc(sizeof(RegisterMsg));
    assert(msg);

    msg->type = (unsigned short) htons(REGISTER);
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

/**
 * Create ErrorFileNotFound message
 * @returns Pointer to newly allocated ErrFileNotFoundMsg struct
 */
ErrFileNotFoundMsg *create_err_file_not_found_msg()
{
    ErrFileNotFoundMsg *msg = 
            (ErrFileNotFoundMsg *)malloc(sizeof(ErrFileNotFoundMsg));
    assert(msg);

    msg->type = (unsigned short) htons(ERR_FILE_NOT_FOUND);
    return msg;
}

/**
 * Create RequestPeer message
 * @param file_name name of file being requested from peer
 * @returns Pointer to newly allocated ErrFileNotFoundMsg struct
 */
ReqPeerMsg *create_reqpeer_msg(std::string file_name)
{
    ReqPeerMsg *msg = (ReqPeerMsg *)malloc(sizeof(ReqPeerMsg));
    assert(msg);

    msg->type = (unsigned short) htons(REQ_PEER);
    bzero(msg->file_name, 20);
    strncpy(msg->file_name, file_name.data(), 20);  

    return msg;
}

/**
 * Create data message that delivers the requested file
 * @param file_sz size of file
 * @param data pointer to data being delivered
 * @returns Pointer to newly allocated data message
 */
char *create_data_msg(int file_sz, char *data)
{
    // Allocate memory for message type, file size, and data
    char *msg = (char *)malloc(file_sz + 6);
    assert(msg);
    unsigned short type = (unsigned short) htons(REQ_PEER);
    memcpy(msg, &type, sizeof(unsigned short));
    int _file_sz = htonl(file_sz);
    memcpy(msg + 2, &_file_sz, sizeof(int));
    memcpy(msg + 6, data, sizeof(char) * file_sz);

    return msg;
}

/*============================= Parse messages =============================*/
/**
 * Parse register message 
 * @param[in] buffer Buffer array containg register message. Buffer size 
 * must be bigger than the size of RegisterMsg struct.
 * @param[out] msg Passed-by-reference value of RegisterMsg struct. The buffer
 * will be parsed and populated in this struct
 * @returns void
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

/**
 * Parse ErrFileNotFound message
 * @param[in] buffer Buffer array containg error message. Buffer size must
 * be bigger than the size of ErrFileNotFoundMsg struct.
 * @param[out] msg Passed-by-reference value of ErrFileNotFoundMsg struct. 
 * The buffer will be parsed and populated in this struct.
 * @returns void
 */
void parse_err_file_not_found_msg(char buffer[], ErrFileNotFoundMsg &msg)
{
    char type_buffer[2];
    memcpy(type_buffer, buffer, 2);
    msg.type = (message_type) ntohs(*(unsigned short *)(type_buffer));

    #ifdef DEBUG_MESSAGE
    std::cout << "\n======================================"
              << "\n\tParsing ERR_FILE_NOT_FOUND"
              << "\n======================================"
              << "\nType: " << msg.type << std::endl;
    #endif
}
