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

/*===========================================================================
 * Peer server core function defintions
 *==========================================================================*/
/* Peer construtor */
Peer::Peer()
{
    char host_buffer[256]; 
    char *IP_buffer; 
    int host_name = gethostname(host_buffer, sizeof(host_buffer)); 
    if (host_name == -1)
    { 
        perror("PEER: gethostname"); 
        exit(EXIT_FAILURE); 
    }
  
    struct hostent *host_entry = gethostbyname(host_buffer); 
    if (host_entry == NULL) 
    { 
        perror("PEER: gethostbyname"); 
        exit(EXIT_FAILURE); 
    }
    IP_buffer = inet_ntoa(*((struct in_addr*) 
                           host_entry->h_addr_list[0])); 
    ip = std::string(IP_buffer);
}

/* Peer destructor */
Peer::~Peer()
{
}

/**
 * Set index server info
 * @param port index server IP
 * @param port index server port
 * @returns void
 */
void Peer::set_index_info(std::string ip, int portno)
{
    index_ip = ip;
    index_portno = portno;
}

/**
 * Start peer server
 * @param port Set peer server port to run on.
 * @returns void
 */
void Peer::start_server(int port) 
{
    portno = port;
    TCP_Select_Server server = TCP_Select_Server(TCP_MAX_NUM_CLIENTS, port);

    /* Server loop */
    while (true) 
    {
        // Initialize socket multiplex
        try 
        {
            server.monitor();
            server.add_sock();
        }
        catch (TCP_Exceptions exception)
        {
            // @TODO: exception handling
            (void) exception;
            std::cout << "Exceptions\n";
        }

        // Handle incoming traffic
        std::vector<SockData> *client_socks = server.get_client_sock_fds();
        for (auto &socket : *client_socks)
        {
            if (server.is_socket_set(socket.sock_fd)) 
            { 
                try 
                {
                    this->handle_incoming_reqs(server, socket);
                }
                catch (TCP_Exceptions exception)
                {
                    (void) exception;
                   // Socket clean-up
                    this->close_and_reset_sock(server, socket);
                }
            }
        }
    }
}

/**
 * Helper function: Wait until socket has buffered enough data as 
 * specified
 * @param sock Passed-by-reference SockData object in client sockets
 * @param server Passed-by-reference TCP_Select_Server
 * @param goal Amount of data that is required to be buffered
 * @returns void
 */
static void finish_buffering(SockData &sock, TCP_Select_Server &server,
                                                        unsigned int goal)
{
    while (sock.bytes_read < goal)
    {
        char buffer[TCP_BUF_SZ];
        int bytes = server.read_from_sock(sock.sock_fd, buffer);
        if (bytes == 0) 
        {
            server.close_sock(sock.sock_fd);
            sock.sock_fd = 0;
            sock.bytes_read = 0;
            bzero(sock.buffer, TCP_BUF_SZ);
            return;
        }

        memcpy(sock.buffer + sock.bytes_read, buffer, bytes);
        sock.bytes_read += bytes;
    }
}

/**
 * Handle incoming requests
 * @param server Passed-by-reference server running in Peer object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Peer::handle_incoming_reqs(TCP_Select_Server &server, SockData &sock)
{
    // Continue buffer message until received at least 2 bytes (messsage type)
    finish_buffering(sock, server, 2);

    // Parse received message
    char type_buf[2];
    memcpy(type_buf, sock.buffer, 2);
    message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));

    // Handle each incoming request to peer server
    switch (t)
    {
        case REQ_FILE:
        {
            // Continue buffering until received full req_file message
            finish_buffering(sock, server, sizeof(ReqFileMsg));
            ReqFileMsg parsed;
            parse_reqfile_msg(sock.buffer, parsed);

            // Send file to peer who is requesting the file
            this->send_file_to_peer(parsed.leecher_ip, 
                                parsed.leecher_portno, parsed.file_name);
            
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case DATA:
        {
            // Continue buffering until received full data message
            finish_buffering(sock, server, sizeof(DataMsg));
            DataMsg parsed;
            parse_data_msg(sock.buffer, parsed);
            this->add_file_segment(parsed);

            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case ERR_FILE_NOT_FOUND:
        {
            std::cerr << "ERR: File requested not found! \n";

            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case FILE_FOUND:
        {
            std::cout << "file found\n";
            break;
        }
        default:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
    }
}

/**
 * Close and reset socket
 * @param server Passed-by-reference server running in Peer object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Peer::close_and_reset_sock(TCP_Select_Server &server, SockData &sock)
{
    server.close_sock(sock.sock_fd);
    sock.sock_fd = 0;
    sock.bytes_read = 0;
    bzero(sock.buffer, TCP_BUF_SZ);
}

/*===========================================================================
 * Request specific function definitions
 *==========================================================================*/

/**
 * Register file to index server. This peer will become a seeder for this 
 * file if it is confirmed
 * @param file_name Name of file being registered
 * @returns void
 */

void Peer::register_file(std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    std::ifstream registered_file(file_name, std::ios::binary);
    try
    {
        if (registered_file)
        {
            // Get file size
            registered_file.seekg(0, registered_file.end);
            int file_size = registered_file.tellg();
            registered_file.seekg(0, registered_file.beg);

            // Get file SHA-256 hash
            std::vector<char> vec(picosha2::k_digest_size);
            std::string file_hash;
            picosha2::hash256(registered_file, vec.begin(), vec.end());
            picosha2::hash256_hex_string(vec, file_hash);

            // Generate register message and send to index server
            RegisterMsg *msg = create_register_msg(
                file_size, file_name, ip, portno, file_hash
            );
            peer_client.connect_to_server(index_ip, index_portno);
            peer_client.write_to_sock((char *)msg, sizeof(RegisterMsg));
            peer_client.close_sock();
            delete msg;

            // File clean-up
            registered_file.close();
        }
        else
        {
            peer_client.close_sock();
        }
    }
    catch (TCP_Exceptions exception)
    {
        if (exception == FAILURE_CONNECT_TO_HOST)
        {
            std::cerr << "ERR: Invalid index server IP and host.. " 
                      << "Hint: Enter index server IP and host again\n";
        }
        else 
        {
            std::cerr << "ERR: Failed to register file '" << file_name
                      << "' to index server\n";
        }
    }
}

/**
 * Send request file message from peer to index
 * @param file_name Name of file being requested
 * @returns void
 */
void Peer::request_file_from_index(std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    try
    {
        ReqFileMsg *reqfile_msg = create_reqfile_msg(file_name, ip, portno);
        peer_client.connect_to_server(index_ip, index_portno);
        peer_client.write_to_sock((char *)reqfile_msg, sizeof(ReqFileMsg));
        peer_client.close_sock();
        delete reqfile_msg;
    }
    catch (TCP_Exceptions exception)
    {
        if (exception == FAILURE_CONNECT_TO_HOST)
        {
            std::cerr << "ERR: Invalid index server IP and host.. " 
                      << "Hint: Enter index server IP and host again\n";
        }
        else 
        {
            std::cerr << "ERR: Failed to request file '" << file_name
                      << "' from index server\n";
        }
    }
}

/**
 * Send request file message from peer to peer
 * @param peer_host IP of peer that has the file
 * @param peer_port Port that peer node (who is possessing the file) is
 * running on
 * @param file_name Name of file being transferred
 * @returns void
 */
void Peer::request_file_from_peer(std::string peer_host, int peer_port,
                                                    std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    try
    {
        ReqFileMsg *reqfile_msg = create_reqfile_msg(file_name, ip, portno);
        peer_client.connect_to_server(peer_host, peer_port);
        peer_client.write_to_sock((char *)reqfile_msg, sizeof(ReqFileMsg));
        peer_client.close_sock();
        delete reqfile_msg;
    }
    catch (TCP_Exceptions exception)
    {
        (void) exception;
        std::cerr << "ERR: Failed to request file '" << file_name << "' from '"
                  << peer_host << ":" << peer_port << "'\n";
    }
}

/**
 * Handle sending file from peer to peer
 * @param peer_host IP of peer that is expecting to receive file
 * @param peer_port Port that peer node (who is expecting to receive file) 
 * is running on
 * @param file_name Name of file being transferred
 * @returns void
 */
void Peer::send_file_to_peer(std::string peer_host, int peer_port,
                                                    std::string file_name)
{
    TCP_Client peer_client = TCP_Client();
    std::ifstream requested_file(file_name, std::ios::binary);
    try
    {
        if (requested_file)
        {
            // Get file size
            requested_file.seekg(0, requested_file.end);
            int file_size = requested_file.tellg();
            requested_file.seekg(0, requested_file.beg);

            // Divide file into 512 bytes buffer chunks for data message
            int segno;
            for (segno = 0; segno < file_size / DATA_MSG_BUF_SIZE; segno++)
            {
                char data[DATA_MSG_BUF_SIZE] = {0};
                requested_file.read(data, DATA_MSG_BUF_SIZE);
                peer_client.connect_to_server(peer_host, peer_port);

                DataMsg *m = create_data_msg(
                    file_size, 
                    DATA_MSG_BUF_SIZE, 
                    segno, file_name, ip, portno, data
                );
                peer_client.write_to_sock((char *)m, sizeof(DataMsg));
                delete m;
                peer_client.close_sock();
            }
            
            // Handle remaining file segment
            int remaining_buf_sz = file_size % DATA_MSG_BUF_SIZE;
            if (remaining_buf_sz != 0)
            {
                char data[DATA_MSG_BUF_SIZE] = {0};
                requested_file.read(data, DATA_MSG_BUF_SIZE);
                peer_client.connect_to_server(peer_host, peer_port);

                DataMsg *m = create_data_msg(
                    file_size,
                    remaining_buf_sz,
                    segno, file_name, ip, portno, data
                );
                peer_client.write_to_sock((char *)m, sizeof(DataMsg));
                delete m;
                peer_client.close_sock();
            }

            // File clean-up
            requested_file.close();
        }
        else
        {
            peer_client.connect_to_server(peer_host, peer_port);
            ErrFileNotFoundMsg *m = create_err_file_not_found_msg();
            peer_client.write_to_sock((char *)m, sizeof(ErrFileNotFoundMsg));
            delete m;
            peer_client.close_sock();
        }
    }
    catch (TCP_Exceptions exception)
    {
        // @TODO: more exception handling
        std::cout << "Request file from peer exception\n";
        (void) exception;
    }
}

/**
 * Add file segment received from DATA message and assemble file once all 
 * segments arrived
 * @param msg Passed-by-reference DataMsg struct containing data segment
 * @returns void
 */
void Peer::add_file_segment(DataMsg &msg)
{
    /** 
     * Data segment table key will be the form of:
     * <seeder ip>:<seeder port>@<file name>
     */
    std::string key = std::string(msg.seeder_ip) + ":" + 
                      std::to_string(msg.seeder_portno) + "@" +
                      std::string(msg.file_name);

    // Handle case when there's only one file segment for entire file
    if (msg.file_size == msg.segment_size)
    {
        std::ofstream downloaded;
        downloaded.open (
            "./files/" + key, 
            std::ios::out | std::ofstream::binary
        );

        if (downloaded.is_open())
        {
            downloaded.write(msg.data, msg.file_size);
            downloaded.close();
            std::cout << "File '" << key << "' finished dowloading\n";
        }
        else
        {
            std::cerr << "Error opening file " << key << "\n";
        }
        return;
    }

    // Handle case when there are more than one segments for entire file
    // Initial insertion to data segment table for provided key
    auto it = segments_table.find(key);
    if (segments_table.find(key) == segments_table.end())
    {
        char *file_ptr = new char[msg.file_size];
        assert(file_ptr);

        char *ptr_to_segno = file_ptr + msg.segno * DATA_MSG_BUF_SIZE;
        memcpy(ptr_to_segno, msg.data, msg.segment_size);
        segments_table[key] = std::pair<int, char*>(
            msg.file_size - msg.segment_size, 
            file_ptr
        );
    }
    // Append more downloaded segment (if any) to entry associated with key
    else
    {
        char *file_ptr = it->second.second;
        assert(file_ptr);

        char *ptr_to_segno = file_ptr + msg.segno * DATA_MSG_BUF_SIZE;
        memcpy(ptr_to_segno, msg.data, msg.segment_size);
        it->second.first -= msg.segment_size;

        // Done downloading
        if (it->second.first == 0)
        {
            std::ofstream downloaded;
            downloaded.open (
                "./files/" + key, 
                std::ios::out | std::ofstream::binary
            );

            if (downloaded.is_open())
            {
                downloaded.write(it->second.second, msg.file_size);
                downloaded.close();
                std::cout << "File '" << key << "' finished dowloading\n";
            }
            else
            {
                std::cerr << "Error opening file " << key << "\n";
            }
            delete[] it->second.second;
            segments_table.erase(key);
        }
    }
}