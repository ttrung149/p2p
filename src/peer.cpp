/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: peer.cpp
 *
 *  Description: Implementation of the PeerServer class declared in `peer.h`.
 *  This class will be used in the `peer-driver.cpp` source file to run the
 *  peer node server.
 *  
 *==========================================================================*/
#include "peer.h"

/* Peer construtor */
Peer::Peer()
{
    char host_buffer[256]; 
    char *IP_buffer; 
    int host_name = gethostname(host_buffer, sizeof(host_buffer)); 
    if (host_name == -1)
    { 
        perror("PEER: gethostname"); 
        exit(EXIT_FAILURE); 
    }
  
    struct hostent *host_entry = gethostbyname(host_buffer); 
    if (host_entry == NULL) 
    { 
        perror("PEER: gethostbyname"); 
        exit(EXIT_FAILURE); 
    }
    IP_buffer = inet_ntoa(*((struct in_addr*) 
                           host_entry->h_addr_list[0])); 
  
    ip = std::string(IP_buffer);
}

/**
 * Start peer server
 * @param name Set peer server name
 * @param port Set pper server port to run on.
 * @returns void
 */
void Peer::start_server(std::string name, int port) 
{
    peer_name = name;
    portno = port;
    TCP_Select_Server server = TCP_Select_Server(TCP_MAX_NUM_CLIENTS, port);

    /* Server loop */
    while (true) 
    {
        // Initialize socket multiplex
        try 
        {
            server.monitor();
            server.add_sock();
        }
        catch (TCP_Exceptions exception)
        {
            // @TODO: exception handling
            (void) exception;
            std::cout << "Exceptions\n";
        }

        std::vector<SockData> *client_socks = server.get_client_sock_fds();
        for (auto &socket : *client_socks)
        {
            if (server.is_socket_set(socket.sock_fd)) 
            { 
                try 
                {
                    this->handle_incoming_reqs(server, socket);
                }
                catch (TCP_Exceptions exception)
                {
                    (void) exception;
                    server.close_sock(socket.sock_fd);
                    socket.sock_fd = 0;
                    socket.bytes_read = 0;
                    bzero(socket.buffer, TCP_BUF_SZ);
                }
            }
        }
    }
}

/**
 * Helper function: Wait until socket has buffered enough data as 
 * specified
 * @param sock Passed-by-reference SockData object in client sockets
 * @param server Passed-by-reference TCP_Select_Server
 * @param goal Amount of data that is required to be buffered
 * @returns void
 */
static void finish_buffering(SockData &sock, TCP_Select_Server &server,
                                                        unsigned int goal)
{
    while (sock.bytes_read < goal)
    {
        char buffer[TCP_BUF_SZ];
        int bytes = server.read_from_sock(sock.sock_fd, buffer);
        if (bytes == 0) 
        {
            server.close_sock(sock.sock_fd);
            sock.sock_fd = 0;
            sock.bytes_read = 0;
            bzero(sock.buffer, TCP_BUF_SZ);
            return;
        }

        memcpy(sock.buffer + sock.bytes_read, buffer, bytes);
        sock.bytes_read += bytes;
    }
}

/**
 * Handle incoming requests
 * @param server Passed-by-reference server running in Peer object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Peer::handle_incoming_reqs(TCP_Select_Server &server, SockData &sock)
{
    // Continue buffer message until received at least 2 bytes (messsage type)
    finish_buffering(sock, server, 2);

    // Parse received message
    char type_buf[2];
    memcpy(type_buf, sock.buffer, 2);
    message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));

    // Handle each incoming request to peer server
    switch (t)
    {
        case REQ_PEER:
        {
            // Continue buffering until received full req_peer message
            finish_buffering(sock, server, sizeof(ReqPeerMsg));
            ReqPeerMsg parsed;
            parse_reqpeer_msg(sock.buffer, parsed);

            // Send file to peer who is requesting the file
            this->send_file_to_peer(parsed.leecher_ip, 
                                parsed.leecher_portno, parsed.file_name);

            server.close_sock(sock.sock_fd);
            sock.sock_fd = 0;
            sock.bytes_read = 0;
            bzero(sock.buffer, TCP_BUF_SZ);
            break;
        }
        case DATA:
        {
            // Continue buffering until received full data message
            finish_buffering(sock, server, sizeof(DataMsg));
            DataMsg parsed;
            parse_data_msg(sock.buffer, parsed);

            server.close_sock(sock.sock_fd);
            sock.sock_fd = 0;
            sock.bytes_read = 0;
            bzero(sock.buffer, TCP_BUF_SZ);
            break;
        }
        case ERR_FILE_NOT_FOUND:
        {
            std::cout << "err\n";
            break;
        }
        case FILE_FOUND:
        {
            std::cout << "file found\n";
            break;
        }
        default:
        {
            std::cout << "default\n";
            server.close_sock(sock.sock_fd);
            sock.sock_fd = 0;
            sock.bytes_read = 0;
            bzero(sock.buffer, TCP_BUF_SZ);
            break;
        }
    }
}

void Peer::request_file_from_peer(std::string peer_host, int peer_port,
                                                    std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    try
    {
        ReqPeerMsg *reqpeer_msg = create_reqpeer_msg(file_name, ip, portno);
        peer_client.connect_to_server(peer_host, peer_port);
        peer_client.write_to_sock((char *)reqpeer_msg, sizeof(ReqPeerMsg));
        peer_client.close_sock();
        delete reqpeer_msg;
    }
    catch (TCP_Exceptions exception)
    {
        // @TODO: more exception handling
        std::cout << "Request file from peer exception\n";
        (void) exception;
    }
}

void Peer::send_file_to_peer(std::string peer_host, int peer_port,
                                                    std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    std::ifstream requested_file(file_name, std::ios::binary);
    try
    {
        if (requested_file)
        {
            // Get file size
            requested_file.seekg(0, requested_file.end);
            int file_size = requested_file.tellg();
            requested_file.seekg(0, requested_file.beg);

            // Divide file into 512 bytes buffer chunks for data message
            int segno;
            for (segno = 0; segno < file_size / DATA_MSG_BUF_SIZE; segno++)
            {
                char data[DATA_MSG_BUF_SIZE] = {0};
                requested_file.read(data, DATA_MSG_BUF_SIZE);
                peer_client.connect_to_server(peer_host, peer_port);

                DataMsg *m = create_data_msg(
                    file_size, 
                    DATA_MSG_BUF_SIZE, 
                    segno, file_name, ip, portno, data
                );
                peer_client.write_to_sock((char *)m, sizeof(DataMsg));
                delete m;
                peer_client.close_sock();
            }
            
            // Handle remaining file segment
            int remaining_buf_sz = file_size % DATA_MSG_BUF_SIZE;
            if (remaining_buf_sz != 0)
            {
                char data[DATA_MSG_BUF_SIZE] = {0};
                requested_file.read(data, DATA_MSG_BUF_SIZE);
                peer_client.connect_to_server(peer_host, peer_port);

                DataMsg *m = create_data_msg(
                    file_size,
                    remaining_buf_sz,
                    segno, file_name, ip, portno, data
                );
                peer_client.write_to_sock((char *)m, sizeof(DataMsg));
                delete m;
                peer_client.close_sock();
            }

            // File clean-up
            requested_file.close();
        }
        else
        {
            std::cout << "File not found\n";
        }
    }
    catch (TCP_Exceptions exception)
    {
        // @TODO: more exception handling
        std::cout << "Request file from peer exception\n";
        (void) exception;
    }

}
