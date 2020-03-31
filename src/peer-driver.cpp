/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: peer-driver.cpp
 *
 *  Description: Main driver for peer node. 
 *  Usage: ./peer
 *  
 *==========================================================================*/
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <csignal>

#include "picosha2.h"
#include "peer.h"

static Peer peer;

void prompt(int signum) {
    std::string prompt;

    std::cout << "\n======================================================"
              << "\nPeer server loop paused. Type 'help' for more details."
              << "\nEnter options: ";
    std::cin >> prompt;

    while (true) {
        if (prompt == "h" || prompt == "help")
        {
            std::cout << "\nAvailable options:"
                      << "\n-----------------"
                      << "\n> q/quit \tQuit peer node"
                      << "\n> req/request \tRequest file from index server"
                      << "\n> reg/register \tRegister file to index server"
                      << "\n> i/index \tUpdate index server information"
                      << std::endl;
            break;
        }
        else if (prompt == "q" || prompt == "quit")
        {
            std::cout << "Exiting.." << std::endl;
            exit(signum);
        }
        else if (prompt == "reg" || prompt == "register")
        {
            std::cout << "Enter name of file being registered: ";
            std::string file_name;
            std::cin >> file_name; 
            peer.register_file(file_name);
            break;
        }
        else if (prompt == "req" || prompt == "request")
        {
            std::cout << "Enter requested file name: ";
            std::string file_name;
            std::cin >> file_name; 
            peer.request_file_from_index(file_name);
            break;
        }
        else if (prompt == "i" || prompt == "index")
        {
            std::string ip, port;
            std::cout << "Update index server information..\n"
                      << "Enter index server IP address: ";
            std::cin >> ip;
            std::cout << "Enter index server port number: ";
            std::cin >> port;
            peer.set_index_info(ip, (unsigned int) std::stoi(port));
            std::cout << "New index server info saved\n";
            break;
        }
        else
        {
            std::cout << "Unknown command.. Try again. Enter options: ";
            std::cin >> prompt;
        }
    }

    std::cout << "\nBack to server loop..\n";
}

/* Main peer driver */
int main(int argc,char* argv[]) 
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./peer <port number>\n";
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, prompt);
    std::string ip, port;
    std::cout << "Enter index server IP address: ";
    std::cin >> ip;
    std::cout << "Enter index server port number: ";
    std::cin >> port;
    peer.set_index_info(ip, (unsigned int) std::stoi(port));

    std::cout << "\n======================================================"
              << "\nStarting peer server at port " << argv[1] << "\n";
    peer.start_server(atoi(argv[1]));

    return 0;
}