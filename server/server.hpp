#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <string>

class Server
{
    public:
        Server(int port_p);
        Server();
        ~Server();
        void boot();
        void run();

    private:
        int sockfd, addrlen, port;
        int const max_clients, bufferSize, bigBufferSize;
        int *client_socket;
        char *buffer;
        std::string *client_name;
        const std::string clientVersion;
        struct fd_set fread;
        sockaddr_in addr;
        // std::map<int, std::string> Usernames;
        
        void sendMsg(int socket,std::string msg,bool isLoged);
        void sendMsg(int socket,std::string msg);
        void check();
        void ops();
        void wlog(std::string ,int enter);
        void wlog(std::string);
        void uploadClientVersionUpToDate(int socket);
        void uploadFile(std::string path, int target);
};

#endif