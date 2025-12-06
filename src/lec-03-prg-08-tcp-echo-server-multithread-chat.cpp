#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

const char* HOST = "127.0.0.1";
const int PORT = 65456;

std::vector<int> group_queue;
std::mutex group_mutex;
std::atomic<int> client_count{0};
std::atomic<bool> running{true};
int serverSocketGlobal = -1;

void handle_client(int clientSocket, sockaddr_in clientAddr) {
    {
        std::lock_guard<std::mutex> lock(group_mutex);
        group_queue.push_back(clientSocket);
        client_count++;
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
        if (msg == "quit") {
            std::lock_guard<std::mutex> lock(group_mutex);
            auto it = std::find(group_queue.begin(), group_queue.end(), clientSocket);
            if (it != group_queue.end()) group_queue.erase(it);
            client_count--;
            break;
        } else {
            std::lock_guard<std::mutex> lock(group_mutex);
            std::cout << "> received ( " << msg << " ) and echoed to "
                      << group_queue.size() << " clients\n";
            for (int conn : group_queue) {
                send(conn, msg.c_str(), msg.size(), 0);
            }
        }
    }

    close(clientSocket);
}

void accept_loop() {
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocketGlobal, (sockaddr*)&clientAddr, &clientLen);
        if (!running) {
            if (clientSocket != -1) close(clientSocket);
            break;
        }
        if (clientSocket == -1) {
            if (!running) break;
            std::perror("accept");
            continue;
        }
        std::thread(handle_client, clientSocket, clientAddr).detach();
    }
}

int main() {
    std::cout << "> echo-server is activated\n";

    serverSocketGlobal = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketGlobal == -1) {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocketGlobal, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(HOST);

    if (bind(serverSocketGlobal, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::perror("bind");
        close(serverSocketGlobal);
        return 1;
    }

    if (listen(serverSocketGlobal, SOMAXCONN) == -1) {
        std::perror("listen");
        close(serverSocketGlobal);
        return 1;
    }

    std::thread server_thread(accept_loop);
    server_thread.detach();

    int baseThreadNumber = 0;

    while (true) {
        std::string msg;
        std::cout << "> ";
        if (!std::getline(std::cin, msg)) break;

        if (msg == "quit") {
            int active = client_count.load();
            if (active == 0) {
                std::cout << "> stop procedure started\n";
                break;
            } else {
                std::cout << "> active threads are remained : "
                          << active << " threads\n";
            }
        }
    }

    running = false;
    close(serverSocketGlobal);

    std::cout << "> echo-server is de-activated\n";
    return 0;
}
