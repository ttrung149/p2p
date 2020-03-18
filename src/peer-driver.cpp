#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "picosha2.h"
#include "peer.h"

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

int main() {
    //std::cout << hash1("main") << std::endl;   
    
    PeerServer peer1 = PeerServer("peer1", 9065);

    return 0;
}
