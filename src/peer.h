#include "tcp.h"

#ifndef PEER_H_
#define PEER_H_

class Peer {
    public:
        Peer();
        void start_server(std::string name, int port);
        void register_file(std::string idx_host, int idx_port, std::string src);
        void request_file();
        
    private:
        std::string peer_name;
};

#endif
