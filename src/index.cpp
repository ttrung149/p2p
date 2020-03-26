/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: index.cpp
 *
 *  Description: Implementation of the Index server class declared in 
 *  `index.h`. This class will be used in the `index-driver.cpp` source file
 *  to run the index node server.
 *  
 *==========================================================================*/
#include "index.h"

/*===========================================================================
 * Index server core function defintions
 *==========================================================================*/
/* Index construtor */
Index::Index()
{
    char host_buffer[256]; 
    char *IP_buffer; 
    int host_name = gethostname(host_buffer, sizeof(host_buffer)); 
    if (host_name == -1)
    { 
        perror("INDEX: gethostname"); 
        exit(EXIT_FAILURE); 
    }
  
    struct hostent *host_entry = gethostbyname(host_buffer); 
    if (host_entry == NULL) 
    { 
        perror("INDEX: gethostbyname"); 
        exit(EXIT_FAILURE); 
    }
    IP_buffer = inet_ntoa(*((struct in_addr*) 
                           host_entry->h_addr_list[0])); 
    ip = std::string(IP_buffer);
}

/**
 * Start index server
 * @param port Set index server port to run on.
 * @returns void
 */
void Index::start_server(int port) 
{
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

        // Handle incoming traffic
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
                   // Socket clean-up
                    this->close_and_reset_sock(server, socket);
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
 * @param server Passed-by-reference server running in Index object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Index::handle_incoming_reqs(TCP_Select_Server &server, SockData &sock)
{
    // Continue buffer message until received at least 2 bytes (messsage type)
    finish_buffering(sock, server, 2);

    // Parse received message
    char type_buf[2];
    memcpy(type_buf, sock.buffer, 2);
    message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));

    // Handle each incoming request to index server
    switch (t)
    {
        case REGISTER:
        {
            // Continue buffering until received full req_file message
            finish_buffering(sock, server, sizeof(RegisterMsg));
            RegisterMsg parsed;
            parse_register_msg(sock.buffer, parsed);

            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case DATA:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case ERR_FILE_NOT_FOUND:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case FILE_FOUND:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        default:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
    }
}

/**
 * Close and reset socket
 * @param server Passed-by-reference server running in Index object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Index::close_and_reset_sock(TCP_Select_Server &server, SockData &sock)
{
    server.close_sock(sock.sock_fd);
    sock.sock_fd = 0;
    sock.bytes_read = 0;
    bzero(sock.buffer, TCP_BUF_SZ);
}