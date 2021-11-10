#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <string>

class Client
{
    public:
        Client(int port_p, std::string serverIP);
        Client();
        ~Client();
        void run();
        std::string getPseudo();
        int getPort();
        void changePseudo();
        void changePort();
        void testDownload();

    private:
        int sockfd, addrlen, port;
        char *buffer;
        int const bufferSize, bigBufferSize;
        std::string _serverIP;
        std::string *clientInfo;
        sockaddr_in addr;
        std::string message;

        void socketBuild();
        void serverConnect();
        void wlog(std::string msg,int enter);
        void wlog(std::string msg);
        void sendMsg(std::string msg, bool isLoged);
        void sendMsg(std::string msg);
        void coms();
        void downloadClientVersionUpToDate();
        void downloadFile(std::string path);
};

#endif