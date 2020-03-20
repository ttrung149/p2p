/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: tcp.h
 *
 *  Description: Function declaration for TCP module. 
 *  The module serves as an wrapper for socket API, and can be used by other 
 *  modules to make TCP connection. 
 *  
 *==========================================================================*/

#include <fstream>
#include <iostream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <netdb.h> 

#ifndef TCP_H_
#define TCP_H_

#define TCP_BUF_SZ 1024

class TCP_Server {
    public:
        TCP_Server(int portno);
        void accept_sock();
        int read_from_sock(char buffer[]);
        int write_to_sock(std::string msg);
        void close_sock();

    private:
        int server_fd, new_socket_fd; 
        struct sockaddr_in server_address;
        int opt = 1;
        int addr_len;
};

class TCP_Client {
    public:
        TCP_Client();
        void connect_to_server(std::string host, int portno);
        int read_from_sock(char buffer[]);
        int write_to_sock(std::string msg);
        void close_sock();

    private:
        int client_fd;
        struct sockaddr_in server_addr;
        struct hostent *server;
};

#endif

