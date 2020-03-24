/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: index-driver.cpp
 *
 *  Description: Main driver for Inde node. 
 *  Usage: ./peer
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
            std::cout << "help" << std::endl;
            break;
        }
        else if (prompt == "q" || prompt == "quit")
        {
            std::cout << "Exiting.." << std::endl;
            exit(signum);
        }
        else if (prompt == "req" || prompt == "request")
        {
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

int main() {
    signal(SIGINT, prompt);
    idx.start_server(9065);

    return 0;
}