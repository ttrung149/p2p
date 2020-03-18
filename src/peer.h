#include <fstream>
#include <iostream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>

#ifndef PEER_H_
#define PEER_H_

class PeerServer {
    public:
        PeerServer(std::string name, int port);
        void register_file();
        void request_file();
        
    private:
        std::string name;
        int port;
        int server_fd, new_socket_fd; 
        struct sockaddr_in address; 
        int opt = 1; 
};

#endif

