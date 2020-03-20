/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: tcp.h
 *
 *  Description: Function definitions for TCP module. 
 *  The module serves as an wrapper for socket API, and can be used by other 
 *  modules to make TCP connection. 
 *  
 *==========================================================================*/

#include "tcp.h"

/*===========================================================================
 * TCP server implementation
 *==========================================================================*/
TCP_Server::TCP_Server(int portno)
{
    addr_len = sizeof(server_address);
       
    /* Creating socket file descriptor */ 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("TCP SERVER ERR: socket"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow server rerun immediately after being killed */   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("TCP SERVER ERR: setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    /* Build the server's Internet address */
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons(portno); 
       
    /* Bind socket to port */
    if (bind(server_fd, (struct sockaddr *)&server_address, 
                                            sizeof(server_address)) < 0)
    { 
        perror("TCP SERVER ERR: bind"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow 5 requests to queue up */
    if (listen(server_fd, 5) < 0) 
    { 
        perror("TCP SERVER ERR: listen"); 
        exit(EXIT_FAILURE); 
    }
}

void TCP_Server::accept_sock() 
{
    /* Generate new socket fd from connection */
    if ((new_socket_fd = accept(server_fd, (struct sockaddr *)&server_address,  
                        (socklen_t*)&addr_len))<0) 
    { 
        perror("TCP SERVER ERR: accept"); 
        exit(EXIT_FAILURE); 
    }
}

int TCP_Server::write_to_sock(std::string msg) 
{
    int bytes_sent = send(new_socket_fd, msg.data(), msg.length(), 0);
    if (bytes_sent < 0)
    { 
        perror("TCP SERVER ERR: write"); 
        exit(EXIT_FAILURE); 
    }
    return bytes_sent;
}

int TCP_Server::read_from_sock(char buffer[]) 
{
    int bytes_read = read(new_socket_fd, buffer, TCP_BUF_SZ);
    if (bytes_read < 0)
    { 
        perror("TCP SERVER ERR: read"); 
        exit(EXIT_FAILURE); 
    }
    return bytes_read;
}

void TCP_Server::close_sock() 
{
    close(new_socket_fd);
}

/*===========================================================================
 * TCP client implementation
 *==========================================================================*/

TCP_Client::TCP_Client()
{
}

void TCP_Client::connect_to_server(std::string host, int portno)
{
    /* Create the socket */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) 
    {
        perror("TCP CLIENT ERR: socket");
    } 

    /* Get the server's DNS entry */
    server = gethostbyname(host.data());
    if (server == NULL) 
    {
        std::cerr << "TCP CLIENT ERR: cannot connect to host '" << host
                  << "'\n'"; 
        exit(EXIT_FAILURE);
    }

    /* Build the server's Internet address */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, 
          (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    /* Connect: create a connection with the server */
    if (connect(client_fd, (struct sockaddr *) &server_addr, 
                                                sizeof(server_addr)) < 0) 
    {
        perror("TCP CLIENT ERR: connect");
        exit(EXIT_FAILURE);
    }
}


int TCP_Client::write_to_sock(std::string msg) 
{
    /* Send message to the server */
    int bytes_sent = write(client_fd, msg.data(), msg.length());
    if (bytes_sent < 0)
    {
        perror("TCP CLIENT ERR: writing to socket");
        exit(EXIT_FAILURE);
    }
    return bytes_sent;
}

int TCP_Client::read_from_sock(char buffer[]) 
{
    int bytes_read = read(client_fd, buffer, TCP_BUF_SZ);
    if (bytes_read < 0)
    { 
        perror("TCP SERVER ERR: read"); 
        exit(EXIT_FAILURE); 
    }
    return bytes_read;
}

void TCP_Client::close_sock() 
{
    close(client_fd);
}
