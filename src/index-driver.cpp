#include "tcp.h"

int main()
{
    TCP_Select_Server server = TCP_Select_Server(TCP_MAX_NUM_CLIENTS, 9065);

    while (true)
    {
        server.monitor();
        server.add_sock();
        std::vector<SockData> *client_sock_fds = server.get_client_sock_fds();
        for (auto &socket : *client_sock_fds)
        {
            if (server.is_socket_set(socket.sock_fd)) 
            { 
                int sd = socket.sock_fd;
                try 
                {
                    char buffer[TCP_BUF_SZ];
                    int bytes_read = server.read_from_sock(sd, buffer);

                    if (bytes_read == 0) 
                    {
                        server.close_sock(sd);
                        sd = 0;
                    }

                    server.write_to_sock(sd, (char *)"Hello from Server!!\n", 21);
                    server.close_sock(sd);
                    socket.sock_fd = 0;
                }
                catch (TCP_Exceptions exception)
                {
                    (void) exception;
                    server.close_sock(sd);
                    socket.sock_fd = 0;
                }
            }
        }
    }  
}