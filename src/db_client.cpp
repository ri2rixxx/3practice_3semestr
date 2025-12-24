#include "db_client.h"
#include <iostream>
#include <poll.h>

DBClient::DBClient(const string& ip, int port) 
    : server_ip(ip), server_port(port), connected(false), sock(-1) {
}

DBClient::~DBClient() {
    disconnect();
}

bool DBClient::connect() {
    if (connected) {
        return true;
    }
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }
    
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(sock);
        sock = -1;
        return false;
    }
    
    if (::connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        sock = -1;
        return false;
    }
    
    // Читаем приветственное сообщение
    char buffer[2048];
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    
    int ret = poll(fds, 1, 1000);
    if (ret > 0 && (fds[0].revents & POLLIN)) {
        recv(sock, buffer, sizeof(buffer) - 1, 0);
    }
    
    connected = true;
    return true;
}

void DBClient::disconnect() {
    if (connected && sock >= 0) {
        close(sock);
        sock = -1;
        connected = false;
    }
}

string DBClient::sendCommand(const string& command) {
    if (!connected) {
        throw runtime_error("Not connected to database");
    }
    
    string cmd = command + "\n";
    int sent = send(sock, cmd.c_str(), cmd.length(), 0);
    if (sent < 0) {
        throw runtime_error("Failed to send command");
    }
    
    // Ждем ответ
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    
    int ret = poll(fds, 1, 3000);
    if (ret <= 0) {
        throw runtime_error("Timeout waiting for response");
    }
    
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        throw runtime_error("Connection closed");
    }
    
    string response(buffer);
    // Убираем \n и \r
    while (!response.empty() && (response.back() == '\n' || response.back() == '\r')) {
        response.pop_back();
    }
    
    return response;
}

string DBClient::hset(const string& table, const string& key, const string& value) {
    string command = "HSET " + table + " " + key + " " + value;
    return sendCommand(command);
}

string DBClient::hget(const string& table, const string& key) {
    string command = "HGET " + table + " " + key;
    return sendCommand(command);
}

bool DBClient::hdel(const string& table, const string& key) {
    string command = "HDEL " + table + " " + key;
    string result = sendCommand(command);
    return result == "УДАЛЕНО";
}
