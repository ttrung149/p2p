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
#include <unordered_map>
#include <utility>
#include "messages.h"
#include "tcp.h"

#ifndef PEER_H_
#define PEER_H_

class Peer 
{
    public:
        Peer();
        void start_server(int);
        void handle_incoming_reqs(TCP_Select_Server &, SockData &);
        void close_and_reset_sock(TCP_Select_Server &, SockData &);

        /* Request specific functions */
        void register_file(std::string, int, std::string);
        void request_file_from_peer(std::string, int, std::string);
        void send_file_to_peer(std::string, int, std::string);
        void add_file_segment(DataMsg &msg);
        
    private:
        std::string ip;
        int portno;
        std::unordered_map<std::string, std::pair<int, char*>> segments_table;
};

#endif
