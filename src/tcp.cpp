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
 * TCP select server implementation
 *==========================================================================*/
TCP_Select_Server::TCP_Select_Server(int _num_clients, int portno)
{
    num_clients = _num_clients;

    SockData init_sockdata = {0, 0, ""};
    /* Initialize client socket descriptor vector */
    if (num_clients < TCP_MAX_NUM_CLIENTS)
    {
        client_sock_fds = std::vector<SockData>(num_clients, init_sockdata);
    }
    else
    {
        client_sock_fds 
            = std::vector<SockData>(TCP_MAX_NUM_CLIENTS, init_sockdata);
    }

    /* Create a master socket */
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
		perror("TCP SELECT SERVER ERR: socket");
        throw FAILURE_CREATE_SOCKET; 
	} 
	
	/* Allow server rerun immediately after being killed */   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("TCP SELECT SERVER ERR: setsockopt"); 
        throw FAILURE_SETSOCKOPT; 
    }

	/* Build the server's Internet address */
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons(portno); 
       
    /* Bind socket to port */
    if (bind(server_fd, (struct sockaddr *)&server_address, 
                                            sizeof(server_address)) < 0)
    { 
        perror("TCP SELECT SERVER ERR: bind"); 
        throw FAILURE_BIND;
    } 
    
    /* Allow 5 requests to queue up */
    if (listen(server_fd, 5) < 0) 
    { 
        perror("TCP SELECT SERVER ERR: listen"); 
        throw FAILURE_LISTEN;
    }
	addr_len = sizeof(server_address);
}

void TCP_Select_Server::monitor()
{
    /* Socket multiplexing setup */
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);

    int max_sd = server_fd;
        
    /* Add child sockets to set */
    for (auto &socket : client_sock_fds) 
    { 
        int sd = socket.sock_fd;
        if(sd > 0) FD_SET(sd, &read_fds);
        if(sd > max_sd) max_sd = sd;
    } 

    /* Wait for an activity on one of the sockets */
    int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) 
    {
        perror("TCP SELECT SERVER ERR: listen"); 
        throw FAILURE_SELECT;
    }
}

void TCP_Select_Server::add_sock()
{
    if (FD_ISSET(server_fd, &read_fds)) 
    { 
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&server_address,
                                    (socklen_t*)&addr_len)) < 0) 
        {
            perror("TCP SELECT SERVER ERR: accept"); 
            throw FAILURE_ACCEPT;
        }
    
        /* Add new socket to vector of client sockets */
        for (auto &socket : client_sock_fds)
        { 
            if (socket.sock_fd == 0) {
                socket.sock_fd = new_socket; 
                break;
            }
        } 
    }
}

bool TCP_Select_Server::is_socket_set(int sock_fd)
{
    return FD_ISSET(sock_fd, &read_fds);
}

std::vector<SockData> *TCP_Select_Server::get_client_sock_fds()
{
    return &client_sock_fds;
}

int TCP_Select_Server::read_from_sock(int sock_fd, char buffer[])
{
    int bytes_read = read(sock_fd, buffer, TCP_BUF_SZ);
    if (bytes_read < 0)
    { 
        perror("TCP SELECT SERVER ERR: reading from socket"); 
        throw FAILURE_READ_SOCK;
    }
    return bytes_read;
}

int TCP_Select_Server::write_to_sock(int sock_fd, char *msg, int size)
{
    int bytes_sent = send(sock_fd, msg, size, 0);
    if (bytes_sent < 0)
    { 
        perror("TCP SELECT SERVER ERR: writing to socket"); 
        throw FAILURE_WRITE_SOCK;
    }
    return bytes_sent;
}

void TCP_Select_Server::close_sock(int sock_fd)
{
    close(sock_fd);
}

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
        throw FAILURE_CREATE_SOCKET; 
    } 
    
    /* Allow server rerun immediately after being killed */   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("TCP SERVER ERR: setsockopt"); 
        throw FAILURE_SETSOCKOPT; 
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
        throw FAILURE_BIND;
    } 
    
    /* Allow 5 requests to queue up */
    if (listen(server_fd, 5) < 0) 
    { 
        perror("TCP SERVER ERR: listen"); 
        throw FAILURE_LISTEN;
    }
}

void TCP_Server::accept_sock() 
{
    /* Generate new socket fd from connection */
    if ((new_socket_fd = accept(server_fd, (struct sockaddr *)&server_address,  
                        (socklen_t*)&addr_len))<0) 
    { 
        perror("TCP SERVER ERR: accept"); 
        throw FAILURE_ACCEPT; 
    }
}

int TCP_Server::write_to_sock(char *msg, int size) 
{
    int bytes_sent = send(new_socket_fd, msg, size, 0);
    if (bytes_sent < 0)
    { 
        perror("TCP SERVER ERR: writing to socket"); 
        throw FAILURE_WRITE_SOCK;
    }
    return bytes_sent;
}

int TCP_Server::read_from_sock(char buffer[]) 
{
    int bytes_read = read(new_socket_fd, buffer, TCP_BUF_SZ);
    if (bytes_read < 0)
    { 
        perror("TCP SERVER ERR: reading from socket"); 
        throw FAILURE_READ_SOCK;
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
        throw FAILURE_CREATE_SOCKET;
    } 

    /* Get the server's DNS entry */
    server = gethostbyname(host.data());
    if (server == NULL) 
    {
        std::cerr << "TCP CLIENT ERR: cannot connect to host '" << host
                  << "'\n'"; 
        throw FAILURE_CONNECT_TO_HOST;
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
        throw FAILURE_CREATE_CONNECTION;
    }
}

int TCP_Client::write_to_sock(char *msg, int size) 
{
    /* Send message to the server */
    int bytes_sent = send(client_fd, msg, size, 0);
    if (bytes_sent < 0)
    {
        perror("TCP CLIENT ERR: writing to socket");
        throw FAILURE_WRITE_SOCK;
    }
    return bytes_sent;
}

int TCP_Client::read_from_sock(char buffer[]) 
{
    int bytes_read = read(client_fd, buffer, TCP_BUF_SZ);
    if (bytes_read < 0)
    { 
        perror("TCP CLIENT ERR: reading from socket"); 
        throw FAILURE_READ_SOCK; 
    }
    return bytes_read;
}

void TCP_Client::close_sock() 
{
    close(client_fd);
}
