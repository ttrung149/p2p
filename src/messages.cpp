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
    RegisterMsg *msg = new RegisterMsg();
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
    ErrFileNotFoundMsg *msg = new ErrFileNotFoundMsg();
    assert(msg);

    msg->type = (unsigned short) htons(ERR_FILE_NOT_FOUND);
    return msg;
}

/**
 * Create Request File message
 * @param file_name name of file being requested
 * @param leecher_ip leecher IP address
 * @param leecher_portno leecher port
 * @returns Pointer to newly allocated ReqFileMsg struct
 */
ReqFileMsg *create_reqfile_msg(std::string file_name, std::string leecher_ip,
                                            unsigned short leecher_portno)
{
    ReqFileMsg *msg = new ReqFileMsg();
    assert(msg);

    msg->type = (unsigned short) htons(REQ_FILE);
    bzero(msg->file_name, 20);
    strncpy(msg->file_name, file_name.data(), 20);

    bzero(msg->leecher_ip, 16);
    strncpy(msg->leecher_ip, leecher_ip.data(), 16);
    msg->leecher_portno = (unsigned short) htons(leecher_portno);

    return msg;
}

/**
 * Create data message that delivers the requested file
 * @param file_sz size of file
 * @param segment_sz size of segment (file will be divided in small chunks)
 * @param segno data segment index of file. Large file will be divided
 * into 512 bytes data segment
 * @param file_name name of file being delivered
 * @param ip IP address of seeder machine holding file segment
 * @param portno Port number that seeder machine holding file segment
 * is running on
 * @param data pointer to data being delivered
 * @returns Pointer to newly allocated data message
 */
DataMsg *create_data_msg(int file_sz, int segment_sz, int segno, 
    std::string file_name, std::string ip, unsigned short portno, char *data)
{
    // Allocate memory for message type, file size, and data
    DataMsg *msg = new DataMsg();
    assert(msg);
    msg->type = (unsigned short) htons(DATA);
    msg->file_size = htonl(file_sz);
    msg->segment_size = htonl(segment_sz);
    msg->segno = htonl(segno);

    bzero(msg->file_name, 20);
    strncpy(msg->file_name, file_name.data(), 20);
    bzero(msg->seeder_ip, 16);
    strncpy(msg->seeder_ip, ip.data(), 16);
    msg->seeder_portno = (unsigned short) htons(portno);

    bzero(msg->data, DATA_MSG_BUF_SIZE);
    memcpy(msg->data, data, sizeof(char) * segment_sz);

    return msg;
}

/*============================= Parse messages =============================*/
/**
 * Parse register message 
 * @param[in] buffer Buffer array containg register message. Buffer size 
 * must be bigger than or equal to the size of RegisterMsg struct.
 * @param[out] msg Passed-by-reference value of RegisterMsg struct. The buffer
 * will be parsed and populated in this struct.
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
 * be bigger than or equal to the size of ErrFileNotFoundMsg struct.
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

/**
 * Parse request file message
 * @param[in] buffer Buffer array containg register message. Buffer size 
 * must be bigger than or equal to the size of ReqFileMsg struct.
 * @param[out] msg Passed-by-reference value of ReqFileMsg struct. The buffer
 * will be parsed and populated in this struct.
 * @returns void
 */
void parse_reqfile_msg(char buffer[], ReqFileMsg &msg)
{
    char type_buffer[2];
    memcpy(type_buffer, buffer, 2);
    msg.type = (message_type) ntohs(*(unsigned short *)(type_buffer));
    strncpy(msg.file_name, buffer + 2, 20);
    strncpy(msg.leecher_ip, buffer + 22, 16);

    char portno_buffer[2];
    memcpy(portno_buffer, buffer + 38, 2);
    msg.leecher_portno = ntohs(*(unsigned short *)(portno_buffer));

    #ifdef DEBUG_MESSAGE
    std::cout << "\n======================================"
              << "\n\tParsing REQ_FILE_MSG"
              << "\n======================================"
              << "\nType: "         << msg.type
              << "\nFile name: "    << msg.file_name 
              << "\nLeecher IP: "   << msg.leecher_ip
              << "\nLeecher port: " << msg.leecher_portno << std::endl;
    #endif
}

/**
 * Parse data message
 * @param[in] buffer Buffer array containg data message. Buffer size 
 * must be bigger than or equal to the size of data message.
 * @param[out] msg Passed-by-reference value of DataMsg struct. The buffer
 * will be parsed and populated in this struct.
 * @returns void
 */
void parse_data_msg(char buffer[], DataMsg &msg)
{
    char type_buffer[2];
    memcpy(type_buffer, buffer, 2);
    msg.type = (message_type) ntohs(*(unsigned short *)(type_buffer));

    char file_sz_buffer[4];
    memcpy(file_sz_buffer, buffer + 2, 4);
    msg.file_size = ntohl(*(unsigned int *)(file_sz_buffer));

    strncpy(msg.file_name, buffer + 6, 20);
    char segment_sz_buffer[4];
    memcpy(segment_sz_buffer, buffer + 26, 4);
    msg.segment_size = ntohl(*(unsigned int *)(segment_sz_buffer));

    char segno_buffer[4];
    memcpy(segno_buffer, buffer + 30, 4);
    msg.segno = ntohl(*(unsigned int *)(segno_buffer));

    strncpy(msg.seeder_ip, buffer + 34, 16);
    char portno_buffer[2];
    memcpy(portno_buffer, buffer + 50, 2);
    msg.seeder_portno = ntohs(*(unsigned short *)(portno_buffer));

    memcpy(msg.data, buffer + 52, DATA_MSG_BUF_SIZE);

    #ifdef DEBUG_MESSAGE
    std::cout << "\n======================================"
              << "\n\tParsing DATA_MSG"
              << "\n======================================"
              << "\nType: "         << msg.type
              << "\nFile size: "    << msg.file_size 
              << "\nFile name: "    << msg.file_name
              << "\nSegment size: " << msg.segment_size 
              << "\nSegment no: "   << msg.segno
              << "\nSeeder IP: "    << msg.seeder_ip
              << "\nSeeder port: "  << msg.seeder_portno
              << "\nData: "         << msg.data << std::endl;
    #endif
}