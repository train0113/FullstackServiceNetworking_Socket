#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

const char* HOST = "127.0.0.1";
const int PORT = 65456;

bool addr_equal(const sockaddr_in& a, const sockaddr_in& b) {
    return a.sin_family == b.sin_family &&
           a.sin_port == b.sin_port &&
           a.sin_addr.s_addr == b.sin_addr.s_addr;
}

int main() {
    std::vector<sockaddr_in> group_queue;

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
        if (msg.empty()) {
            continue;
        }

        char clientIP[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        int clientPort = ntohs(clientAddr.sin_port);

        if (msg[0] == '#' || msg == "quit") {
            if (msg == "#REG") {
                std::cout << "> client registered " << clientIP << ":" << clientPort << "\n";
                auto it = std::find_if(group_queue.begin(), group_queue.end(),
                                       [&](const sockaddr_in& s) { return addr_equal(s, clientAddr); });
                if (it == group_queue.end()) {
                    group_queue.push_back(clientAddr);
                }
            } else if (msg == "#DEREG" || msg == "quit") {
                auto it = std::find_if(group_queue.begin(), group_queue.end(),
                                       [&](const sockaddr_in& s) { return addr_equal(s, clientAddr); });
                if (it != group_queue.end()) {
                    std::cout << "> client de-registered " << clientIP << ":" << clientPort << "\n";
                    group_queue.erase(it);
                }
            }
        } else {
            if (group_queue.empty()) {
                std::cout << "> no clients to echo\n";
            } else {
                auto it = std::find_if(group_queue.begin(), group_queue.end(),
                                       [&](const sockaddr_in& s) { return addr_equal(s, clientAddr); });
                if (it == group_queue.end()) {
                    std::cout << "> ignores a message from un-registered client\n";
                } else {
                    std::cout << "> received ( " << msg << " ) and echoed to "
                              << group_queue.size() << " clients\n";
                    for (const auto& addr : group_queue) {
                        sendto(serverSocket,
                               msg.c_str(),
                               msg.size(),
                               0,
                               (const sockaddr*)&addr,
                               sizeof(addr));
                    }
                }
            }
        }
    }

    close(serverSocket);
    std::cout << "> echo-server is de-activated\n";
    return 0;
}
