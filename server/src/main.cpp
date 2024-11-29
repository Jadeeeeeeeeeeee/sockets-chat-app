#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <thread>
#include <vector>

#pragma comment(lib, "WS2_32")

std::vector<SOCKET> clients; // here store all socket and clients

void reciveMsg();

int main() {
    // init the dll
    WSADATA wsadata;
    WORD wVersionRequestd = MAKEWORD(2, 2);
    int wsaerr = WSAStartup(wVersionRequestd, &wsadata);
    if (wsaerr != 0) {
        std::cerr << "winsock dll was not found\n";
        return 1;
    }

    // creating server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // set up struct of ip and info
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4369);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binding the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //listening for new clients
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for client connection..." << std::endl;

    // accept client connections in a loop
    std::thread t_recv(reciveMsg);
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue; // Try accepting again
        }

        // add the new client socket to the list
        clients.push_back(clientSocket);
        char buffer[1024] = { 0 };
        recv(clientSocket, buffer, sizeof(buffer)-1, 0);
        std::string msg = std::string(buffer) + " has joined";
        for(size_t i = 0; i < clients.size(); i++) {
                int name = send(clients[i], msg.c_str(), strlen(msg.c_str()), 0);
        }
    }

    if (t_recv.joinable())
        t_recv.join();

    for (SOCKET clientSocket : clients) {
        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

// function to receive messages from all clients
void reciveMsg() {
    char buffer[1024] = { 0 };
    fd_set readSet; // socket activity is here

    while (true) {
        if (clients.empty()) {
            Sleep(50);  // small delay if no clients
            continue;
        }

        // we add the sockets to the activity set
        FD_ZERO(&readSet);
        for (const auto& socket : clients) {
            FD_SET(socket, &readSet);
        }

        // check for socket activity
        timeval timeout = { 0, 50000 };  // 50ms timeout
        int activity = select(0, &readSet, nullptr, nullptr, &timeout);
        // if theres a error disply it
        if (activity == SOCKET_ERROR) {
            std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        // check each client for messages
        for (size_t i = 0; i < clients.size(); i++) {
            if (FD_ISSET(clients[i], &readSet)) {  // only check sockets with data
                int bytesReceived = recv(clients[i], buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::cout << buffer << std::endl;
                    int client = i;
                    //go through all clients and send them this msg
                    for (size_t x = 0; x < clients.size(); x++) {
                        if(clients[x] != clients[i]){
                            int sendResult = send(clients[x], buffer, strlen(buffer), 0);
                        }
                    }
                }
                else if (bytesReceived == 0) {
                    std::cout << "Client " << i + 1 << " disconnected" << std::endl;
                    closesocket(clients[i]);
                    clients.erase(clients.begin() + i);
                    i--;
                }
                else {
                    // recive failed so we just delete the client
                    std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
                    closesocket(clients[i]);
                    clients.erase(clients.begin() + i);
                    i--;
                }
            }
        }
    }
}