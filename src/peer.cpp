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
    peer_portno = port;
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
static bool finished_buffering(SockData &sock, int goal, int bytes_read,
                                char buffer[])
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
        memcpy(sock.buffer + sock.bytes_read, buffer, _bytes_read);
        sock.bytes_read += _bytes_read;

        char type_buf[2];
        memcpy(type_buf, buffer, 2);
        message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));
    
        switch (t)
        {
            case REQ_PEER:
            {
                if (!finished_buffering(sock, sizeof(ReqPeerMsg), 
                                                _bytes_read, buffer)) return;
                std::cout << "req_peer\n";
                
                ReqPeerMsg parsed_reqpeer;
                parse_reqpeer_msg(sock.buffer, parsed_reqpeer);
                break;
            }
            
            case DATA:
                std::cout << "data\n";
                break;
            
            case ERR_FILE_NOT_FOUND:
                std::cout << "err\n";
                break;
            
            case FILE_FOUND:
                std::cout << "file found\n";
                break;

            default:
                server.close_sock(sd);
                sock.sock_fd = 0;
                break;
        }
    }

    // server.write_to_sock(sd, (char *)"Hello from Server!!\n", 21);
    // server.close_sock(sd);
    // sock.sock_fd = 0;
}

void Peer::register_file(std::string idx_host,
                                    int idx_port, std::string src)
{
    TCP_Client peer_client = TCP_Client();

    try 
    {
        ReqPeerMsg *reqpeer_msg = create_reqpeer_msg("file.txt");
        peer_client.connect_to_server(idx_host, idx_port);
        peer_client.write_to_sock((char *)reqpeer_msg, sizeof(ReqPeerMsg));

        // char buffer[1024] = {0};
        // peer_client.read_from_sock(buffer);

        // std::cout << "Response from server: " << buffer << std::endl;
        peer_client.close_sock();
        free(reqpeer_msg);
    }
    catch (TCP_Exceptions exception)
    {
        std::cout << "Client exception\n";
        (void) exception;
    }

    (void) src;
}
