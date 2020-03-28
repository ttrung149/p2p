/*============================================================================
 *  p2p - Trung Truong
 *
 *  File name: index.cpp
 *
 *  Description: Implementation of the Index server class declared in 
 *  `index.h`. This class will be used in the `index-driver.cpp` source file
 *  to run the index node server.
 *  
 *==========================================================================*/
#include "index.h"

/*===========================================================================
 * Index server core function defintions
 *==========================================================================*/
/* Index construtor */
Index::Index()
{
    char host_buffer[256]; 
    char *IP_buffer; 
    int host_name = gethostname(host_buffer, sizeof(host_buffer)); 
    if (host_name == -1)
    { 
        perror("INDEX: gethostname"); 
        exit(EXIT_FAILURE); 
    }
  
    struct hostent *host_entry = gethostbyname(host_buffer); 
    if (host_entry == NULL) 
    { 
        perror("INDEX: gethostbyname"); 
        exit(EXIT_FAILURE); 
    }
    IP_buffer = inet_ntoa(*((struct in_addr*) 
                           host_entry->h_addr_list[0])); 
    ip = std::string(IP_buffer);
}

/**
 * Start index server
 * @param port Set index server port to run on.
 * @returns void
 */
void Index::start_server(int port) 
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
 * @param server Passed-by-reference server running in Index object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Index::handle_incoming_reqs(TCP_Select_Server &server, SockData &sock)
{
    // Continue buffer message until received at least 2 bytes (messsage type)
    finish_buffering(sock, server, 2);

    // Parse received message
    char type_buf[2];
    memcpy(type_buf, sock.buffer, 2);
    message_type t = (message_type) ntohs(*(unsigned short *)(type_buf));

    // Handle each incoming request to index server
    switch (t)
    {
        case REQ_FILE:
        {
            // Continue buffering until received full req_file message
            finish_buffering(sock, server, sizeof(ReqFileMsg));
            ReqFileMsg parsed;
            parse_reqfile_msg(sock.buffer, parsed);
            this->send_seeder_info(parsed.leecher_ip, parsed.leecher_portno, 
                                                            parsed.file_name);
            
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case REGISTER:
        {
            // Continue buffering until received full register message
            finish_buffering(sock, server, sizeof(RegisterMsg));
            RegisterMsg parsed;
            parse_register_msg(sock.buffer, parsed);

            // Send confirmation to decide to accept or reject registered file
            this->confirm_file(parsed);

            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case REGISTER_ACK:
        {
            // Continue buffering until received full register message
            finish_buffering(sock, server, sizeof(RegisterAckMsg));
            RegisterAckMsg parsed;
            parse_register_ack_msg(sock.buffer, parsed);

            // Update file entry table based on received ACK message
            this->update_file_entry_table(parsed);

            // Socket clean-up
            this->close_and_reset_sock(server, sock);
            break;
        }
        case FILE_FOUND:
        {
            // Socket clean-up
            this->close_and_reset_sock(server, sock);
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
 * @param server Passed-by-reference server running in Index object
 * @param sock Passed-by-reference SockData object in client sockets
 * vector
 * @returns void
 */
void Index::close_and_reset_sock(TCP_Select_Server &server, SockData &sock)
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
 * Allows index server to query file in file entry table for monitoring
 * @param file_name file being querried
 * @returns void
 */
void Index::query_file(std::string file_name)
{
    auto it = file_entry_table.find(file_name);
    if (it == file_entry_table.end())
    {
        std::cerr << "File '" << file_name << "' not found!\n";
    }
    else
    {
        std::cout << "File hash: " << std::string(it->second.file_hash) 
                  << "\nSeeder addrs: \n";
        for (auto &addr : it->second.seeders_addr)
        {
            std::cout << "> " << addr.first << ":" << addr.second << "\n";
        }
    }
}

/**
 * Send confirmation message to confirm whether file being registered exists.
 * File information received through RegisterMsg is store in pending file 
 * entry table.
 * @param parsed Passed-by-reference RegisterMsg
 * @returns void
 */
void Index::confirm_file(RegisterMsg &parsed)
{
    TCP_Client index_client = TCP_Client();
    index_client.connect_to_server(parsed.seeder_ip, parsed.seeder_portno);
    RegisterConfirmMsg *m = create_reg_confirm_msg(
        parsed.file_size, std::string(parsed.file_name), ip, portno
    );

    // Add file to pending file table
    std::string key = std::string(parsed.seeder_ip) + ":" + 
                      std::to_string(parsed.seeder_portno) + "@" + 
                      std::string(parsed.file_name);
    memcpy(pending_file_table[key], parsed.file_hash, 64);

    // Send register confirm message to client
    index_client.write_to_sock((char *)m, sizeof(RegisterConfirmMsg));
    delete m;
    index_client.close_sock();
}

/**
 * Update file entry table based on received RegisterAckMsg
 * @param parsed Passed-by-reference RegisterAckMsg
 * @returns void
 */
void Index::update_file_entry_table(RegisterAckMsg &parsed)
{
    // Move pending file entry to confirmed file table
    std::string key = std::string(parsed.seeder_ip) + ":" +
                      std::to_string(parsed.seeder_portno) + "@" +
                      std::string(parsed.file_name);

    // Find pending entry
    auto pending_it = pending_file_table.find(key);
    if (pending_it == pending_file_table.end())
    {
        std::cerr << "ERR: Failed to transfer file '" 
                  << std::string(parsed.file_name)
                  << "' from pending to confirmed file table\n";
        return;
    }

    // Compare pending entry hash to received hash
    if (memcmp(pending_it->second, parsed.file_hash, 64) == 0)
    {
        // First seeder added to file entry table
        auto it = file_entry_table.find(std::string(parsed.file_name));
        if (it == file_entry_table.end())
        {
            FileEntry e;
            memcpy(e.file_hash, parsed.file_hash, 64);
            e.seeders_addr.push_back(
                std::pair<std::string, unsigned short> (
                    std::string(parsed.seeder_ip),
                    int(parsed.seeder_portno)
                )
            );
            file_entry_table[std::string(parsed.file_name)] = e;
        }
        // More seeders added to file entry table
        else
        {      
            it->second.seeders_addr.push_back(
                std::pair<std::string, unsigned short> (
                    std::string(parsed.seeder_ip),
                    int(parsed.seeder_portno)
                )
            );
        }
        std::cout << "File '" << std::string(parsed.file_name)
                  << "' is added to file table by seeder '"
                  << std::string(parsed.seeder_ip) << ":" 
                  << std::to_string(parsed.seeder_portno) << "'\n";
    }
    else
    {
        std::cerr << "ERR: Failed to transfer file '" 
                    << std::string(parsed.file_name)
                    << "' from pending to confirmed\n";
        std::cerr << "Hint: Hash does not match\n";
    }

    pending_file_table.erase(key);
}

/**
 * Send seeder info to peer node based on entries in file entry table
 * @param leecher_ip IP address of leecher peer
 * @param leecher_portno Port that leecher peer node is running on
 * @param file_name Name of file being requested
 * @returns void
 */
void Index::send_seeder_info(std::string leecher_ip, int leecher_portno,
                                                    std::string file_name)
{
    TCP_Client index_client = TCP_Client();
    auto it = file_entry_table.find(file_name);
    if (it == file_entry_table.end())
    {
        index_client.connect_to_server(leecher_ip, leecher_portno);
        ErrFileNotFoundMsg *m = create_err_file_not_found_msg();
        index_client.write_to_sock((char *)m, sizeof(ErrFileNotFoundMsg));
        delete m;
    }
    else
    {
        FileEntry e = it->second;

        // Randomly select a seeder
        srand(time(NULL));
        int random_idx = rand() % e.seeders_addr.size();
        index_client.connect_to_server(leecher_ip, leecher_portno);
        FileFoundMsg *m = create_file_found_msg(
            file_name, 
            e.seeders_addr[random_idx].first,
            e.seeders_addr[random_idx].second,
            std::string(e.file_hash)
        );
        index_client.write_to_sock((char *)m, sizeof(FileFoundMsg));
        delete m;
    }
    index_client.close_sock();
}
