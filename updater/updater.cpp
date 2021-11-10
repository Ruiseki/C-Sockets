#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include "windows.h"
#include <sys/time.h>

using namespace std;

void wlog(string text)
{
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    ofstream write("./log.txt", ios::app);
    write<<timeinfo->tm_hour<<':'<<timeinfo->tm_min<<':'<<timeinfo->tm_sec<<':'<<msecs_time%1000<<'\t';
    write<<text<<endl;
}

void sendMsg(int master_socket, string msg)
{
    int success = send(master_socket,msg.c_str(),msg.size(),0);
    if(success < 0)
    {
        wlog("[UPDATER] Failed to send :\""+msg+"\"");
        throw new exception();
    }
}

void downloadFile(int master_socket, string path)
{
    int valread, streamSize, fileSize;
    int const bigBufferSize(1024 * 1024),bufferSize(1024);
    char *buffer;
    ofstream output(path.c_str(), ios::binary);

    Sleep(2000);
    wlog("[UPDATER] Sending //ready");
    send(master_socket, "//ready", strlen("//ready"), 0); // ready to download
    cout<<"-> ready"<<endl;
    
    wlog("[UPDATER] Waiting //ready..."); // waiting the server
    buffer = new char[bufferSize];
    if(recv(master_socket,buffer,bufferSize,0) < 0)
    {
        wlog("Failed receiving");
        throw new exception();
    }
    cout<<"<- ready"<<endl;
    
    delete[] buffer; buffer=0;
    buffer = new char[bufferSize];

    wlog("[UPDATER] Waiting fileSize...");
    valread = recv(master_socket,buffer,bufferSize,0);
    buffer[valread] = '\0';
    cout<<"Valread : "<<valread<<endl<<buffer<<endl;

    sendMsg(master_socket,"//next");

    delete[] buffer; buffer=0;
    wlog("[UPDATER] Starting download");
    do
    {
        buffer = new char[bufferSize];
        valread = recv(master_socket,buffer,sizeof(buffer),0); // streamSize reception
        buffer[valread] = '\0';
        if(string(buffer) == "//last")
        {
            sendMsg(master_socket,"//next"); // ready for the next operation
            valread = recv(master_socket,buffer,bufferSize,0);// final streamSize reception
            buffer[valread] = '\0';

            streamSize = stoi((string)buffer); // operational streamSize
            sendMsg(master_socket,"//next"); // ready for the next operation
            delete[] buffer; buffer=0;
            buffer = new char[bigBufferSize];
            recv(master_socket,buffer,bigBufferSize,0); // final buffer reception
            output.write(buffer,streamSize);
            sendMsg(master_socket,"//end");
            delete[] buffer; buffer=0;
            break;
        }
        streamSize = stoi(string(buffer)); // operational streamSize
        sendMsg(master_socket,"//next"); // ready for the next operation

        delete[] buffer; buffer=0;
        buffer = new char[bigBufferSize];
        recv(master_socket,buffer,bigBufferSize,0); // binary data reception
        output.write(buffer,streamSize); // writing binary data into the output
        delete[] buffer; buffer=0;

        sendMsg(master_socket,"//next"); // ready for the next operation
    }while(true);
}

int main(int argc, char *argv[])
{
    int master_socket, valread, port, addrlen;
    string serverAddr;
    WSADATA ws;
    int const bufferSize(1024);
    char *buffer;
    sockaddr_in addr;

    port = 55000;
    serverAddr = "93.16.2.231";

    if(argc > 1)
    {
        if(argc == 2) port = stoi(argv[1]);
        else
        {
            port = stoi(argv[1]);
            serverAddr = argv[2];
        }
    }

    addrlen = sizeof(addr);

    cout << "Shuting down the old client" << endl;
    system("TASKKILL /F /IM client.exe");
    Sleep(2000);
    cout << "Deleting the old client" << endl;
    system("DEL client.exe");
    Sleep(2000);

    if(WSAStartup(MAKEWORD(2,2),&ws) < 0) {cout<<"Error : WSADATA"<<endl; return -1;}

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    memset(&(addr.sin_zero),0,8);
    addr.sin_addr.s_addr = inet_addr(serverAddr.c_str());

    if(addr.sin_addr.s_addr <= 0) {cout<<"Error : server adress"<<endl; return -1;}

    master_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(master_socket < 0) {cout<<"Error : master_socket declaration"<<endl; return -1;}

    while(connect(master_socket,(sockaddr*)&addr,addrlen) < 0)
    {
        cout<<"Connection to the server impossible. Retry in 5 seconds"<<endl;
        Sleep(5000);
    }

    send(master_socket,"//name=UPDATER",strlen("//name=UPDATER"),0);

    send(master_socket,"//updater",strlen("//updater"),0);

    cout<<"Downloading .exe"<<endl;
    wlog("[UPDATER] Downloading .exe");
    downloadFile(master_socket,"./client.exe");
    cout<<"Done"<<endl<<endl;
    
    /* cout<<"Downloading .bat"<<endl;
    wlog("[UPDATER] Downloading .bat");
    downloadFile(master_socket,"./updatingclient.bat");
    cout<<"Done"<<endl<<endl; */

    buffer = new char[bufferSize];

    valread = recv(master_socket,buffer,bufferSize,0);
    buffer[valread] = '\0';

    ifstream readData("./data.txt");
    string text;

    if(!readData) ofstream("./data.txt") << "User" << endl << string(buffer);
    else 
    {
        getline(readData,text);
        ofstream("./data.txt") << text << endl << string(buffer);
    }

    delete[] buffer; buffer=0;

    shutdown(master_socket,2);

    string command("client.exe ");
    command += port;
    command += " ";
    command += serverAddr;

    system(command.c_str());
    
    return 0;
}