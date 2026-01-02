#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <lz4.h>

class Connection {
    public:
        int sock;
        sockaddr_in serverAddr;
        Connection(std::string server_ip, int server_port);
        void connect_();
        ~Connection() {
            close(sock);
        }
        void sendFrame(uint8_t* data, int data_size);
};