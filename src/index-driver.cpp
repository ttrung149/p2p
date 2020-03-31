/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: index-driver.cpp
 *
 *  Description: Main driver for Index node. 
 *  Usage: ./index <port number>
 * 
 *==========================================================================*/
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <csignal>

#include "picosha2.h"
#include "index.h"

static Index idx;

void prompt(int signum) {
    std::string prompt;

    std::cout << "\n======================================================"
              << "\nIndex server loop paused. Type 'help' for more details."
              << "\nEnter options: ";
    std::cin >> prompt;

    while (true) {
        if (prompt == "h" || prompt == "help")
        {
            std::cout << "\nAvailable options:"
                      << "\n-----------------"
                      << "\n> q/quit \tQuit index node"
                      << "\n> query \tQuery file from entry table"
                      << std::endl;
            break;
        }
        else if (prompt == "q" || prompt == "quit")
        {
            std::cout << "Exiting.." << std::endl;
            exit(signum);
        }
        else if (prompt == "query")
        {
            std::cout << "Enter query file name: ";
            std::string file_name;
            std::cin >> file_name; 
            idx.query_file(file_name);
            break;
        }
        else
        {
            std::cout << "Unknown command.. Try again" << std::endl;
            std::cin >> prompt;
        }
    }

    std::cout << "Back to server loop.." << std::endl;
}

/* Index driver */
int main(int argc,char* argv[]) {
    if (argc != 2)
    {
        std::cerr << "Usage: ./index <port number>\n";
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, prompt);
    std::cout << "\n======================================================"
              << "\nStarting index server at port " << argv[1] << "\n";
    idx.start_server(atoi(argv[1]));

    return 0;
}
