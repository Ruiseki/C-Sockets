#include "client.hpp"
#include <winsock2.h>
#include <string>
#include <fstream>
#include <iostream>
#include "windows.h"
using namespace std;

Client::Client(int port_p, string serverIP) : bufferSize(1024), bigBufferSize(1024 * 1024)
{
    _serverIP = serverIP;
    clientInfo = new string[2]; // 2 client info : Name, version

    this->wlog("\n\n ##STARTING THE CLIENT##",2);
    buffer = 0;
    addrlen = sizeof(addr);
    port = port_p;
    clientInfo[0] = "User";
    this->wlog("Port set at : "+to_string(port));
    this->wlog("Server IP set at : "+_serverIP);
    this->wlog("Reading data.txt...");
    ifstream dataRead("./data.txt");

    string text;
    if(dataRead)
    {
        getline(dataRead,text);
        this->wlog("File found. Pseudo : "+text);
        clientInfo[0] = text;
        getline(dataRead,text);
        this->wlog("Version : "+text);
        clientInfo[1]=text;
    }
    else
    {
        this->wlog("File not found. Creating a new one...");
        clientInfo[1]="0.0.0";
        ofstream("./data.txt")<<clientInfo[0]<<endl<<clientInfo[1];
        this->wlog("Done. Pseudo : "+clientInfo[0]);
    }
}

Client::Client() : Client(55000,"93.16.2.231")
{
    
}

Client::~Client()
{
    delete[] clientInfo; clientInfo=0;
    delete[] buffer;
}

void Client::run()
{
    system("cls");
    cin.ignore();
    this->socketBuild();
    this->serverConnect();
    // this->coms();

    string message;
    buffer = new char[bufferSize];
    while(message != "//close")
    {
        getline(cin,message);
        if(message != "//close")
        {
            this->sendMsg(message);
        }
    }
    delete[] buffer; buffer=0;
    shutdown(sockfd,2);  // shutdown : termine une connection.
                            // dernier parametre :
                            // 0 = mettre fin aux opération de récéptions
                            // 1 = mettre fin aux opération d'envoies
                            // 2 = mettre fin aux deux
}

void Client::socketBuild()
{
    WSADATA ws;//Initialiser le WSA, je sais pas c'est kwa
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0)
    {
        //erreur
        //impossible d'initialiser le WSA
        this->wlog("WSA - MERDE!");
        throw new exception();
    }this->wlog("WSA - OK!");

    
    addr.sin_port = htons(port); // toujours penser à traduire le port en réseau
    addr.sin_family = AF_INET; // notre socket est TCP
    memset(&(addr.sin_zero),0,8);
    addr.sin_addr.s_addr = inet_addr(_serverIP.c_str()); // Indique l'adresse du serveur

    if(addr.sin_addr.s_addr <= 0)
    {
        // erreur
        // impossible d'ouvrir le socket
        this->wlog("ADRESSE - MERDE!");
        throw new exception();
    }this->wlog("ADRESSE - OK!");

    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); // déclaration du socket

    if(sockfd < 0)
    {
        // erreur
        // impossible d'ouvrir le socket
        this->wlog("SOCKET - MERDE!");
        throw new exception();
    }else this->wlog("SOCKET - OK!");
}

void Client::serverConnect()
{
    int valread;
    string text;

    if(connect(sockfd,(sockaddr*)&addr,sizeof(addr)) < 0)
    {
        this->wlog("CONNECT - MERDE!");
        throw new exception();
    }this->wlog("CONNECT - OK!");

    this->sendMsg("//name="+clientInfo[0]);

    buffer = new char[bufferSize];
    recv(sockfd,buffer,bufferSize,0);
    this->wlog("The server get the client's name");
    delete[] buffer;

    buffer = new char[bufferSize];
    
    this->sendMsg("//version="+clientInfo[1]);
    
    valread = recv(sockfd,buffer,bufferSize,0);
    buffer[valread] = '\0';
    text=buffer;
    if(text=="//cheak")
    {
        this->wlog("Server -> //check");
        this->wlog("Client Version is good");
    }
    else if(text=="//needDl")
    {
        this->wlog("Server -> //needDl");
        this->wlog("Need to download the new version");
        // this->downloadClientVersionUpToDate();
        string command("updater.exe ");
        command += port;
        command += " ";
        command += _serverIP;
        system(command.c_str());
        shutdown(sockfd,2);
        exit(0);
    }
}

void Client::coms()
{

}

void Client::wlog(string msg, int enter)
{
    ofstream write("./log.txt", ios::app);
    write<<msg;
    for(int i(0);i<enter;i++) write<<endl;
}

void Client::wlog(string msg)
{
    this->wlog(msg,1);
}

void Client::sendMsg(string msg, bool isLoged)
{
    send(sockfd,msg.c_str(),msg.size(),0);
    if(isLoged) this->wlog("Message send : "+msg);
}

void Client::sendMsg(string msg)
{
    this->sendMsg(msg,true);
}

string Client::getPseudo()
{
    return clientInfo[0];
}

int Client::getPort()
{
    return port;
}

void Client::changePseudo()
{
    system("cls");
    cin.ignore();
    cout<<"New pseudo : ";
    getline(cin,clientInfo[0]);
    ofstream file("./data.txt");
    file<<clientInfo[0];
}

void Client::changePort()
{
    system("cls");
    cout<<"New port : ";
    cin>>port;
}

void Client::downloadClientVersionUpToDate()
{
    string newerVersionPath("./newClient.exe"),
    updateBat("./updatingclient.bat"),
    text;
    system("cls");

    this->wlog("Starting download \"newClient.exe\"...");
    cout<<"Starting download \"newClient.exe\"..."<<endl;
    this->downloadFile(newerVersionPath);                   // downloading the new version
    this->wlog("Download completed !",2);
    cout<<"Download completed !"<<endl;

    this->wlog("Starting download \"updatingclient.bat\"...");
    cout<<"Starting download \"updatingclient.bat\"..."<<endl;
    this->downloadFile(updateBat);                          // downloading the bat file for "installation"
    this->wlog("Download completed !",2);
    cout<<"Download completed !"<<endl;

    this->wlog("Changing the version number");              

    buffer = new char[bufferSize];
    int valread;

    valread = recv(sockfd,buffer,bufferSize,0);
    buffer[valread] = '\0';
    text = buffer;

    this->wlog("New version : "+text);
    ofstream("./data.txt") << clientInfo[0] << endl << text ;

    delete[] buffer;
    buffer=0;

    shutdown(sockfd,2);
    system("start updatingclient.bat");
}

void Client::downloadFile(string path)
{
    int valread, streamSize, 
    fileSize, totalPacket, i(0), progression, progressionOld(-1);

    // progression : current progression in % of the download
    // progressionOld : old progression. We are going to update the display every 1% for optimisation.

    ofstream output(path.c_str(), ios::binary);
    char data[bigBufferSize],buf[bufferSize];

    this->sendMsg("//ready",false); // ready to download
    this->wlog("Waiting the //ready...");
    if(recv(sockfd,buf,bufferSize,0) < 0)
    {   
        wlog("Failed receiving");
        throw new exception();
    }

    valread = recv(sockfd,buf,bufferSize,0);
    buf[valread] = '\0';
    fileSize = stoi(string(buf));

    totalPacket = fileSize / bigBufferSize + 1;

    this->sendMsg("//next");

    this->wlog("Starting download...");
    do
    {
        i++; // packet number
        progression = i * 100 / totalPacket; 
        // just for rappel : we use a int, so,
        // "progression > progressionOld" only 101 time in the loop (because 0% count)

        if(progression > progressionOld)
        {
            system("cls");
            cout<<"Downloading..."<<endl<<"[";
            int x;
            x = progression / 10;
            for(int i(0); i < x; i++) cout<<"*";
            for(x; x < 10; x++) cout<<" ";
            cout<<"] "<<progression<<"%"<<endl;
            progressionOld = progression;
        }

        valread = recv(sockfd,buf,bufferSize,0); // streamSize reception
        buf[valread] = '\0';


        if(string(buf) == "//last")
        {
            this->sendMsg("//next",false); // ready for the next operation
            valread = recv(sockfd,buf,bufferSize,0); // final streamSize reception
            buf[valread] = '\0';

            this->wlog("nop");
            streamSize = stoi((string)buf); // operational streamSize
            this->sendMsg("//next",false); // ready for the next operation
            recv(sockfd,data,bigBufferSize,0); // final buffer reception
            output.write(data,streamSize);
            this->sendMsg("//end",false);
            break;
        }
        this->wlog("ok");
        streamSize = stoi(string(buf)); // operational streamSize
        this->sendMsg("//next",false); // ready for the next operation

        recv(sockfd,data,bigBufferSize,0); // binary data reception
        output.write(data,streamSize); // writing binary data into the output

        this->sendMsg("//next",false); // ready for the next operation
    }while(true);

    this->wlog("Download completed !",2);
}

void Client::testDownload()
{
    this->socketBuild();
    this->serverConnect();
    Sleep(1000);
    this->sendMsg("//testDownload");

    string path,response;
    ifstream isExist;
    int val;
    char buf[bufferSize];
    cin.ignore();
    do
    {
        cout<<"File to client : ";
        getline(cin,path);
        this->sendMsg(path);
        val = recv(sockfd,buf,bufferSize,0);
        buf[val] = '\0';
        response = buf;
        this->wlog("Server -> "+response);
    }while(response=="//retry");

    this->downloadFile("./file."+path.substr(path.size()-3,path.size()));
    shutdown(sockfd,2);
}