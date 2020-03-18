#include "peer.h"

PeerServer::PeerServer(std::string server_name, int server_port) {
    name = server_name;
    port = server_port;

    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    const std::string msg = "Hello from server\n"; 
       
    /* Creating socket file descriptor */ 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("PEER SERVER ERR: socket"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow server rerun immediately after being killed */   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("PEER SERVER ERR: setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    /* Build the server's Internet address */
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(server_port); 
       
    /* Bind socket to port */
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    { 
        perror("PEER SERVER ERR: bind"); 
        exit(EXIT_FAILURE); 
    } 
    
    /* Allow 5 requests to queue up */
    if (listen(server_fd, 5) < 0) 
    { 
        perror("PEER SERVER ERR: listen"); 
        exit(EXIT_FAILURE); 
    } 

    /* Generate new socket fd from connection */
    if ((new_socket_fd = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("PEER SERVER ERR: accept"); 
        exit(EXIT_FAILURE); 
    }

    int valread = read (new_socket_fd, buffer, 1024); 
    (void) valread;
    std::cout << "Buffer:" << buffer << std::endl;   

    send(new_socket_fd, msg.data(), msg.length(), 0); 
    std::cout << "Message sent\n";
}
