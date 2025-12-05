#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;

    std::cout << "> echo-client is activated\n";

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::perror("socket");
        std::cout << "> socket() failed and program terminated\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(HOST);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::perror("connect");
        std::cout << "> connect() failed and program terminated\n";
        close(clientSocket);
        std::cout << "> echo-client is de-activated\n";
        return 1;
    }

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (true) {
        std::cout << "> ";
        std::string sendMsg;
        if (!std::getline(std::cin, sendMsg)) {
            break;
        }

        if (send(clientSocket, sendMsg.c_str(), sendMsg.size(), 0) == -1) {
            std::perror("send");
            break;
        }

        int recvLen = recv(clientSocket, buffer, BUF_SIZE, 0);
        if (recvLen <= 0) {
            std::cout << "> server closed the connection\n";
            break;
        }

        std::string recvMsg(buffer, recvLen);
        std::cout << "> received: " << recvMsg << "\n";

        if (sendMsg == "quit") {
            break;
        }
    }

    close(clientSocket);

    std::cout << "> echo-client is de-activated\n";
    return 0;
}
