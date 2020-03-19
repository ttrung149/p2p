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

/* PeerServer construtor */
PeerServer::PeerServer() 
{
}

void PeerServer::start_server(std::string name, int port) 
{
    peer_name = name;
    peer_port = port;

    /* Peer server setup */
    int server_fd, new_socket_fd; 
    struct sockaddr_in address; 
    int opt = 1;
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    const std::string msg = "Hello from server\n"; 
       
    /* Creating socket file descriptor */ 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("PEER SERVER ERR: socket"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow server rerun immediately after being killed */   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("PEER SERVER ERR: setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    /* Build the server's Internet address */
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port); 
       
    /* Bind socket to port */
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    { 
        perror("PEER SERVER ERR: bind"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow 5 requests to queue up */
    if (listen(server_fd, 5) < 0) 
    { 
        perror("PEER SERVER ERR: listen"); 
        exit(EXIT_FAILURE); 
    } 

    /* Server loop */
    while (true) 
    {
        std::cout << "Server loop" << std::endl;

        /* Generate new socket fd from connection */
        if ((new_socket_fd = accept(server_fd, (struct sockaddr *)&address,  
                           (socklen_t*)&addrlen))<0) 
        { 
            perror("PEER SERVER ERR: accept"); 
            exit(EXIT_FAILURE); 
        }

        int bytes_read = read(new_socket_fd, buffer, 1024); 
        (void) bytes_read;
        std::cout << "Buffer:" << buffer << std::endl;   

        int bytes_sent = send(new_socket_fd, msg.data(), msg.length(), 0); 
        std::cout << "Message sent\n";
        (void) bytes_sent;
        
        close(new_socket_fd);
    }
}

/* Function: register_file 
 * Params:
 *
 **/
void PeerServer::register_file(std::string idx_host,
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

