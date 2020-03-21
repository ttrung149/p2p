/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: peer.cpp
 *
 *  Description: Implementation of the PeerServer class declared in `peer.h`.
 *  This class will be used in the `peer-driver.cpp` source file to run the
 *  peer node server.
 *  
 *==========================================================================*/
#include "peer.h"

/* Peer construtor */
Peer::Peer()
{
}

void Peer::start_server(std::string name, int port) 
{
    peer_name = name;
    TCP_Server peer_server = TCP_Server(port);

    /* Server loop */
    while (true) 
    {
        char buffer[1024] = {0};
        peer_server.accept_sock();

        peer_server.read_from_sock(buffer);
        std::cout << "Buffer: " << buffer << std::endl;   

        peer_server.write_to_sock("Hello from peer server\n");
        peer_server.close_sock();
    }
}

void Peer::register_file(std::string idx_host,
                                    int idx_port, std::string src)
{
    TCP_Client peer_client = TCP_Client();
    
    peer_client.connect_to_server(idx_host, idx_port);
    peer_client.write_to_sock("Hello from peer client\n");

    char buffer[1024] = {0};
    peer_client.read_from_sock(buffer);

    std::cout << "Response from server: " << buffer << std::endl;
    peer_client.close_sock();

    (void) src;
}
