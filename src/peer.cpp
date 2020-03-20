/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: peer.cpp
 *
 *  Description: Implementation of the PeerServer class defined in `peer.h`.
 *  This class will be used in the `peer-driver.cpp` source file to run the
 *  peer node server.
 *  
 *==========================================================================*/
#include "peer.h"

/* Peer construtor */
Peer::Peer()
{
}

void Peer::start_server(std::string name, int port) 
{
    peer_name = name;
    TCP_Server peer_server = TCP_Server(port);

    /* Server loop */
    while (true) 
    {
        char buffer[1024] = {0};

        std::cout << "Server loop" << std::endl;
        peer_server.accept_sock();

        peer_server.read_from_sock(buffer);
        std::cout << "Buffer:" << buffer << std::endl;   

        peer_server.write_to_sock("Hello from peer server\n");
        peer_server.close_sock();
    }
}

/* Function: register_file 
 * Params:
 *
 **/
void Peer::register_file(std::string idx_host,
                                    int idx_port, std::string src)
{
    int client_fd;
    struct sockaddr_in idx_server_addr;
    struct hostent *idx_server;
    char buf[1024] = "Hello from peer client";

    index_server_host = idx_host;
    index_server_port = idx_port;

    /* Create the socket */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) 
    {
        perror("PEER CLIENT ERR: socket");
    } 

    /* Get the server's DNS entry */
    idx_server = gethostbyname(idx_host.data());
    if (idx_server == NULL) 
    {
        std::cerr << "PEER CLIENT ERR: cannot connect to host '" << idx_host
                  << "'\n'"; 
        exit(EXIT_FAILURE);
    }

    /* Build the server's Internet address */
    bzero((char *) &idx_server_addr, sizeof(idx_server_addr));
    idx_server_addr.sin_family = AF_INET;
    bcopy((char *) idx_server->h_addr, 
          (char *) &idx_server_addr.sin_addr.s_addr, idx_server->h_length);
    idx_server_addr.sin_port = htons(idx_port);

    /* Connect: create a connection with the server */
    if (connect(client_fd, (struct sockaddr *) &idx_server_addr, 
                                                sizeof(idx_server_addr)) < 0) 
    {
        perror("PEER CLIENT ERR: connect");
        exit(EXIT_FAILURE);
    }

    /* Send message to the server */
    int bytes_sent = write(client_fd, buf, strlen(buf));
    if (bytes_sent < 0)
    {
        perror("PEER CLIENT ERR: writing to socket");
        exit(EXIT_FAILURE);
    } 

    /* Print the server's reply */
    bzero(buf, 1024);
    int bytes_read = read(client_fd, buf, 1024);
    if (bytes_read < 0)
    {
        perror("PEER CLIENT ERR: reading from socket");
        exit(EXIT_FAILURE);
    }

    std::cout << "Response from server: " << buf << std::endl;
    close(client_fd);

    (void) src;
}

