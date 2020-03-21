/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: peer.h
 *
 *  Description: Declerations of the PeerServer class.
 *  This class will be used in the `peer-driver.cpp` source file to run the
 *  peer node server.
 *  
 *==========================================================================*/
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
