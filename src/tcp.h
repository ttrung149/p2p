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
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <netdb.h>
#include <vector>

#ifndef TCP_H_
#define TCP_H_

/* Constants */
const int TCP_BUF_SZ = 1024;
const int TCP_MAX_NUM_CLIENTS = 50;

enum TCP_Exceptions {
    FAILURE_SETSOCKOPT,
    FAILURE_BIND,
    FAILURE_LISTEN,
    FAILURE_ACCEPT,
    FAILURE_SELECT,
    FAILURE_CREATE_SOCKET,
    FAILURE_CONNECT_TO_HOST,
    FAILURE_CREATE_CONNECTION,
    FAILURE_WRITE_SOCK,
    FAILURE_READ_SOCK
};

/* TCP_Select_Server class */
typedef struct socket_data {
    int sock_fd;
    int bytes_read;
    char buffer[TCP_BUF_SZ];
} SockData;

class TCP_Select_Server 
{
    public:
        TCP_Select_Server(int num_clients, int portno);
        void monitor();
        void add_sock();
        int read_from_sock(int sock_fd, char buffer[]);
        int write_to_sock(int sock_fd, char *msg, int size);
        void close_sock(int sock_fd);
        bool is_socket_set(int sock_fd);

        /* Getters */
        std::vector<SockData> *get_client_sock_fds();

    private:
        int server_fd;
        int num_clients;
        std::vector<SockData> client_sock_fds;
        struct sockaddr_in server_address;
        int opt = 1;
        int addr_len;
        fd_set read_fds;
};

/* TCP_Server class */
class TCP_Server 
{
    public:
        TCP_Server(int portno);
        void accept_sock();
        int read_from_sock(char buffer[]);
        int write_to_sock(char *msg, int size);
        void close_sock();

    private:
        int server_fd, new_socket_fd; 
        struct sockaddr_in server_address;
        int opt = 1;
        int addr_len;
};

/* TCP_Client class */
class TCP_Client 
{
    public:
        TCP_Client();
        void connect_to_server(std::string host, int portno);
        int read_from_sock(char buffer[]);
        int write_to_sock(char *msg, int size);
        void close_sock();

    private:
        int client_fd;
        struct sockaddr_in server_addr;
        struct hostent *server;
};

#endif

