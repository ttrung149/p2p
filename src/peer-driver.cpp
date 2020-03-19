#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <csignal>

#include "picosha2.h"
#include "peer.h"

static PeerServer peer;

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

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program
    peer.register_file("130.64.23.182", 9065, "file.txt");

    exit(signum);
}

int main() {
    //std::cout << hash1("main") << std::endl;   
    signal(SIGINT, signalHandler);   
    peer.start_server("peer1", 9065);

    return 0;
}
