#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

const char* HOST = "127.0.0.1";
const int PORT = 65456;

void handle_client(int clientSocket, sockaddr_in clientAddr) {
    char clientIP[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
    int clientPort = ntohs(clientAddr.sin_port);

    std::cout << "[thread] client connected by IP address "
              << clientIP << " with Port number " << clientPort << "\n";

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (true) {
        int recvLen = recv(clientSocket, buffer, BUF_SIZE, 0);
        if (recvLen <= 0) {
            std::cout << "[thread] client disconnected\n";
            break;
        }

        std::string msg(buffer, recvLen);
        std::cout << "[thread] echoed: " << msg << "\n";

        if (send(clientSocket, msg.c_str(), msg.size(), 0) == -1) {
            std::perror("send");
            break;
        }

        if (msg == "quit") {
            std::cout << "[thread] client sent quit, closing connection\n";
            break;
        }
    }

    close(clientSocket);
    std::cout << "[thread] connection closed\n";
}

int main() {
    std::cout << "> echo-server (multithread) is activated\n";

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(HOST);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::perror("bind");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == -1) {
        std::perror("listen");
        close(serverSocket);
        return 1;
    }

    while (true) {
        std::cout << "> waiting for a new client...\n";

        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            std::perror("accept");
            continue;
        }

        std::thread(handle_client, clientSocket, clientAddr).detach();
    }

    close(serverSocket);
    std::cout << "> echo-server (multithread) is de-activated\n";
    return 0;
}
