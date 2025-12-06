#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;

    std::cout << "> echo-server is activated\n";

    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        std::perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(HOST);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::perror("bind");
        close(serverSocket);
        return 1;
    }

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int recvLen = recvfrom(serverSocket,
                               buffer,
                               BUF_SIZE,
                               0,
                               (sockaddr*)&clientAddr,
                               &clientLen);
        if (recvLen <= 0) {
            continue;
        }

        std::string msg(buffer, recvLen);
        std::cout << "> echoed: " << msg << "\n";

        if (sendto(serverSocket,
                   msg.c_str(),
                   msg.size(),
                   0,
                   (sockaddr*)&clientAddr,
                   clientLen) == -1) {
            std::perror("sendto");
        }
    }

    close(serverSocket);
    std::cout << "> echo-server is de-activated\n";
    return 0;
}
