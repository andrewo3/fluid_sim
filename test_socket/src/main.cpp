#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <lz4.h>
#include <vector>


int WIDTH = 256;
int HEIGHT = 256;

int main(int argc, char** argv) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) { perror("socket"); return 1; }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(65432);           // Receiver port
    inet_pton(AF_INET, "192.168.193.25", &serverAddr.sin_addr); // Receiver IP

    printf("Connecting to server...\n");
    if(connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect"); return 1;
    }
    printf("Connected!\n");

    char test[WIDTH*HEIGHT*4];
    for (int i = 0; i < WIDTH*HEIGHT*4; i++) {
        if (i % 4 == 3) {
            test[i] = 255;
        } else if (i%4 == 0) {
            test[i] = 255; //R
        } else if (i%4 == 1) {
            test[i] = 0;   //G
        } else {
            test[i] = 255;   //B
        }
    }

    int maxCompressedSize = LZ4_COMPRESSBOUND(WIDTH*HEIGHT*4);
    std::vector<char> compressed(maxCompressedSize);

    printf("Compressing frame...\n");
    int compressedSize = LZ4_compress_default(
        (const char*)test,
        compressed.data(),
        WIDTH*HEIGHT*4,
        maxCompressedSize
    );

    if(compressedSize <= 0) {
        std::cerr << "Compression failed\n";
        return 1;
    }

    printf("Sending frame...\n");
    // --- 4. Send frame size first ---
    uint32_t sizeNet = htonl(compressedSize); // network byte order
    if(send(sock, &sizeNet, sizeof(sizeNet), 0) != sizeof(sizeNet)) {
        perror("send size"); return 1;
    }

    // --- 5. Send compressed data ---
    int totalSent = 0;
    while(totalSent < compressedSize) {
        int sent = send(sock, compressed.data() + totalSent, compressedSize - totalSent, 0);
        if(sent <= 0) { perror("send data"); return 1; }
        totalSent += sent;
    }

    std::cout << "Sent compressed frame of " << compressedSize << " bytes\n";

    close(sock);
    return 0;
}