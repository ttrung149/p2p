/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: index.h
 *
 *  Description: Declerations of the Index Server class.
 *  This class will be used in the `index-driver.cpp` source file to run the
 *  index node server.
 *  
 *==========================================================================*/
#include <unordered_map>
#include <utility>
#include "messages.h"
#include "tcp.h"

#ifndef INDEX_H_
#define INDEX_H_

typedef struct file_entry {
    char file_hash[64];
    std::vector<std::pair<char[16], unsigned short>> seeders_addr;
} FileEntry;

class Index 
{
    public:
        Index();
        void start_server(int);
        void handle_incoming_reqs(TCP_Select_Server &, SockData &);
        void close_and_reset_sock(TCP_Select_Server &, SockData &);

        /* Request specific functions */
        void confirm_registered_file();
        void confirm_report();
        void send_seeder_peer_info();

    private:
        std::string ip;
        int portno;
        std::unordered_map<std::string, FileEntry> file_entry_table;
};

#endif
