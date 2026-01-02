#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <lz4.h> 

class Connection {
    public:
        int sock;
        sockaddr_in serverAddr;
        Connection(std::string server_ip, int server_port);
        void connect_();
        ~Connection();
        void sendFrame(uint8_t* data, int data_size);
};