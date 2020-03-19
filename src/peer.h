#include <fstream>
#include <iostream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <netdb.h> 

#ifndef PEER_H_
#define PEER_H_

class PeerServer {
    public:
        PeerServer();
        void start_server(std::string name, int port);
        
        void register_file(std::string idx_host, int idx_port, std::string src);
        void request_file();
        
    private:
        std::string peer_name;
        int peer_port;

        std::string index_server_host;
        int index_server_port;
};

#endif

