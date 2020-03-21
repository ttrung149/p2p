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

std::string hash1(std::string fileName)
{
    std::ifstream inputFile(fileName, std::ios::binary);
    std::vector<char> hashVec(picosha2::k_digest_size);
    std::string hexHash;

    picosha2::hash256(inputFile, hashVec.begin(), hashVec.end());
    picosha2::hash256_hex_string(hashVec, hexHash);
    inputFile.close();
    
    return hexHash;
}

void prompt(int signum) {
    std::string prompt;

    std::cout << "\n======================================================"
              << "\nPeer server loop paused. Type 'help' for more details."
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
        else if (prompt == "r" || prompt == "register")
        {
            peer.register_file("130.64.23.181", 9065, "file.txt");
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
    //std::cout << hash1("main") << std::endl;   
    signal(SIGINT, prompt);
    peer.start_server("peer1", 9065);

    return 0;
}