#ifndef DB_CLIENT_H
#define DB_CLIENT_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

using namespace std;

class DBClient {
private:
    int sock;
    string server_ip;
    int server_port;
    bool connected;
    
public:
    DBClient(const string& ip = "127.0.0.1", int port = 6379);
    ~DBClient();
    
    bool connect();
    void disconnect();
    string sendCommand(const string& command);

    string hset(const string& table, const string& key, const string& value);
    string hget(const string& table, const string& key);
    bool hdel(const string& table, const string& key);
};

#endif
