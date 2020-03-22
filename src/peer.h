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
#include "messages.h"
#include "tcp.h"

#ifndef PEER_H_
#define PEER_H_

class Peer 
{
    public:
        Peer();
        void start_server(std::string, int);
        void handle_incoming_reqs(TCP_Select_Server &, SockData &);
        void register_file(std::string, int, std::string);
        
    private:
        std::string peer_name;
        int peer_portno;
};

#endif
