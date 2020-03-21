#include "tcp.h"

int main()
{
    TCP_Select_Server server = TCP_Select_Server(TCP_MAX_NUM_CLIENTS, 9065);

    while (true)
    {
        server.monitor();
        server.add_sock();
        std::vector<int> *client_sock_fds = server.get_client_sock_fds();
        for (auto &sd : *client_sock_fds)
        {
            if (server.is_socket_set(sd)) { 
                char buffer[1024];
                
                int bytes_read = server.read_from_sock(sd, buffer);
                if (bytes_read == 0) {
                    server.close_sock(sd);
                    sd = 0;
                }

                server.write_to_sock(sd, (char *)"Hello from Server!!\n", 21);
                server.close_sock(sd);
                sd = 0;
            }
        }
    }

    // fd_set *read_fds = server.get_read_fds();

    // while (true)
    // {
    //     
}