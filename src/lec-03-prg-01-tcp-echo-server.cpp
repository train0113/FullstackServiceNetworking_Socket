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

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(serverSocket, 1) == -1) {
        std::perror("listen");
        close(serverSocket);
        return 1;
    }

    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
    if (clientSocket == -1) {
        std::perror("accept");
        close(serverSocket);
        return 1;
    }

    char clientIP[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
    int clientPort = ntohs(clientAddr.sin_port);

    std::cout << "> client connected by IP address "
              << clientIP << " with Port number " << clientPort << "\n";

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (true) {
        int recvLen = recv(clientSocket, buffer, BUF_SIZE, 0);
        if (recvLen <= 0) {
            break;
        }

        std::string msg(buffer, recvLen);
        std::cout << "> echoed: " << msg << "\n";

        send(clientSocket, msg.c_str(), msg.size(), 0);

        if (msg == "quit") {
            break;
        }
    }

    close(clientSocket);
    close(serverSocket);

    std::cout << "> echo-server is de-activated\n";
    return 0;
}
