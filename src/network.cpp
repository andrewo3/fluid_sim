#include "network.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <lz4.h>   // Assuming LZ4 is still being used

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

Connection::Connection(std::string server_ip, int server_port) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        sock = INVALID_SOCKET;
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed with error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port); // Use the port parameter

    // Convert IP string to binary form
    if (inet_pton(AF_INET, server_ip.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address\n";
        closesocket(sock);
        WSACleanup();
        sock = INVALID_SOCKET;
        return;
    }
}

void Connection::connect_() {
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed with error: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        sock = INVALID_SOCKET;
        return;
    }
}

void Connection::sendFrame(uint8_t* data, int data_size) {
    if (sock == INVALID_SOCKET) return;

    int maxCompressedSize = LZ4_compressBound(data_size);
    std::vector<char> compressed(maxCompressedSize);

    // Compress frame
    int compressedSize = LZ4_compress_default(
        (const char*)data,
        compressed.data(),
        data_size,
        maxCompressedSize
    );

    if (compressedSize <= 0) {
        std::cerr << "Failed to compress\n";
        return;
    }

    // Send compressed size as header
    uint32_t sizeNet = htonl(compressedSize); // network byte order
    if (send(sock, (const char*)&sizeNet, sizeof(sizeNet), 0) != sizeof(sizeNet)) {
        std::cerr << "Failed to send size\n";
        return;
    }

    // Send compressed frame
    int totalSent = 0;
    while (totalSent < compressedSize) {
        int sent = send(sock, compressed.data() + totalSent, compressedSize - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "Failed to send data: " << WSAGetLastError() << "\n";
            return;
        }
        totalSent += sent;
    }
}

// Destructor to clean up
Connection::~Connection() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}
