#include "network.hpp"

Connection::Connection(std::string server_ip, int server_port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) { perror("socket"); return; }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(65432);           // Receiver port
    inet_pton(AF_INET, server_ip.c_str(), &serverAddr.sin_addr); // Receiver IP
}

void Connection::connect_() {
    if(connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect"); return;
    }   
}

void Connection::sendFrame(uint8_t* data, int data_size) {
    int maxCompressedSize = LZ4_COMPRESSBOUND(data_size);
    std::vector<char> compressed(maxCompressedSize);

    //compress frame
    int compressedSize = LZ4_compress_default(
        (const char*)data,
        compressed.data(),
        data_size,
        maxCompressedSize
    );

    if(compressedSize <= 0) {
        perror("failed to compress");
        return;
    }

    //send compressed size as header
    uint32_t sizeNet = htonl(compressedSize); // network byte order
    if(send(sock, &sizeNet, sizeof(sizeNet), 0) != sizeof(sizeNet)) {
        perror("send size"); return;
    }

    //send compressed frame
    int totalSent = 0;
    while(totalSent < compressedSize) {
        int sent = send(sock, compressed.data() + totalSent, compressedSize - totalSent, 0);
        if(sent <= 0) { perror("send data"); return; }
        totalSent += sent;
    }
}