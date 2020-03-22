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
 * Helper function: Check if socket has buffered enough data as 
 * specified
 * @param sock Passed-by-reference SockData object in client sockets
 * @param goal Amount of data that is required to be buffered
 * @param bytes_read Recently bytes read amount from socket
 * @param buffer Buffer array recently read from socket
 * @returns Returns true if the buffer size in SockData is equal to
 * or exceeds goal. Else, returns false.
 */
static bool finished_buffering(SockData &sock, unsigned int goal, 
                                unsigned int bytes_read, char buffer[])
{
    if (sock.bytes_read + bytes_read < goal)
    {
        memcpy(sock.buffer + sock.bytes_read, buffer, bytes_read);
        sock.bytes_read += bytes_read;
        return false;
    }
    return true;
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
    char buffer[TCP_BUF_SZ] = {0};
    int sd = sock.sock_fd;
    int _bytes_read = server.read_from_sock(sd, buffer);

    // Closes and reset socket when there's no more to read
    if (_bytes_read == 0) 
    {
        server.close_sock(sd);
        sock.sock_fd = 0;
        sock.bytes_read = 0;
        bzero(sock.buffer, TCP_BUF_SZ);
    }
    // Continue buffer message until received at least 2 bytes (messsage type)
    else if (!finished_buffering(sock, 2, _bytes_read, buffer)) return;
    // Parse received message
    else 
    {
        // Update buffer in sock
        memcpy(sock.buffer + sock.bytes_read, buffer, _bytes_read);
        sock.bytes_read += _bytes_read;

        char type_buf[2];
        memcpy(type_buf, buffer, 2);
        message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));
    
        // Handle each incoming request to peer server
        switch (t)
        {
            case REQ_PEER:
            {
                if (!finished_buffering(sock, sizeof(ReqPeerMsg), 0, buffer)) 
                {
                    return;
                }
                std::cout << "req_peer\n";
                
                // Parse received peer file request message
                ReqPeerMsg parsed;
                parse_reqpeer_msg(sock.buffer, parsed);

                // Send file to peer who is requesting the file
                this->send_file_to_peer(parsed.leecher_ip, 
                                    parsed.leecher_portno, parsed.file_name);
                break;
            }
            
            case DATA:
            {
                std::cout << "data\n";
                // Continue buffering until received msg type and file size
                if (!finished_buffering(sock, 6, 0, buffer)) return;
                DataMsg parsed;
                parse_data_msg(sock.buffer, parsed);

                char *data_received = new char[parsed.file_size + 6];
                memcpy(data_received, sock.buffer, sock.bytes_read);

                while (sock.bytes_read < parsed.file_size + 6)
                {
                    bzero(sock.buffer, TCP_BUF_SZ);
                    int bytes = server.read_from_sock(sock.sock_fd, buffer);
                    memcpy(data_received + sock.bytes_read, buffer, bytes);
                    sock.bytes_read += bytes;
                }

                parse_data_msg(data_received, parsed);
                server.close_sock(sd);
                sock.sock_fd = 0;
                sock.bytes_read = 0;
                bzero(sock.buffer, TCP_BUF_SZ);

                delete[] data_received;
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
                server.close_sock(sd);
                sock.sock_fd = 0;
                sock.bytes_read = 0;
                bzero(sock.buffer, TCP_BUF_SZ);
                break;
            }
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

    // File handling
    std::ifstream requested_file(file_name, std::ios::binary);
    try
    {
        if (requested_file)
        {
            // Get file size
            requested_file.seekg (0, requested_file.end);
            int file_size = requested_file.tellg();
            requested_file.seekg (0, requested_file.beg);

            // Read in file to buffer
            char *data = new char[file_size];
            assert(data);
            requested_file.read(data, file_size);

            // Send data message
            char *data_msg = create_data_msg(file_size, data);
            peer_client.connect_to_server(peer_host, peer_port);
            peer_client.write_to_sock(data_msg, file_size + 6);
            peer_client.close_sock();
            
            delete[] data_msg;
            delete[] data;
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
