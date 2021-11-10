#include "server.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <map>
#include "windows.h"
#include <sys/time.h>
#include <thread>

using namespace std;

Server::Server(int port_p) : bufferSize(1024), bigBufferSize(1024 * 1024), max_clients(5), clientVersion("a.0.15")
{
    client_socket = new int[max_clients];
    client_name = new string[max_clients];
    buffer = 0;
    addrlen = sizeof(addr);
    port = port_p;
}

Server::Server() : Server(55000)
{
    
}

Server::~Server()
{
    delete[] client_socket; client_socket=0;
    delete[] client_name; client_name=0;
    delete[] buffer; buffer=0;
}

void Server::boot()
{
    this->wlog("\n\n### STARTING THE SERVER ###",2);
    WSADATA ws; // Initialiser le WSA, c'est un truc à windaube lié aux sockets mais je sais pas c'est kwa
    if(WSAStartup(MAKEWORD(2,2),&ws) < 0){
        //erreur : impossible d'initialiser le WSA
        wlog("WSA - MERDE!");
        throw new exception();
    }else wlog("WSA - OK!");

    addr.sin_addr.s_addr = INADDR_ANY; // indique que toutes les sources seront acceptées
    addr.sin_port = htons(port); // toujours penser à traduire le port en réseau
    addr.sin_family = AF_INET; // notre socket est TCP
    memset(&(addr.sin_zero),0,8); // je sais pas c'est kwa

    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); // déclaration du socket
    if(sockfd < 0){
        // erreur : impossible d'ouvrir le socket
        wlog("SOCKET - MERDE!");
        throw new exception();
    }else wlog("SOCKET - OK!");

    // int setsockopt()  option du socket
    int res = bind(sockfd,(sockaddr*)&addr,sizeof(addr));
    if(res < 0)
    {
        // erreur : impossible de se caller sur le port donné
        wlog("BIND - MERDE!");
        throw new exception();
    } else wlog("BIND - OK!"); 
}

void Server::run()
{
    int lis = listen(sockfd,max_clients); // max_clients = backlog. Nmbr de personne pouvant demander des trucs simultanément au serv
    if(lis < 0)
    {
        // erreur : impossible d'écouter sur le port donné
        wlog("LISTEN - MERDE!");
        throw new exception();
    } else wlog("LISTEN - OK!");
   
    for(int i(0) ; i < max_clients ; i++) client_socket[i]=0; // Initialise clients_socket

    cout<<"Lancement du serveur sur le port "<<port<<endl;
    while(true)
    {
        // cout<<"Checking..."<<endl;
        this->check();
        // cout<<"Treating..."<<endl;
        this->ops();
    }
}

void Server::check()
{
    /* FD_ZERO(&fwrite);
    FD_ZERO(&ferror);

    FD_SET(sockfd,&fread);
    FD_SET(sockfd,&ferror); */
    int sd, max_sd, activity;
    struct timeval tv;
    tv.tv_sec=1;
    tv.tv_usec=0;

    FD_ZERO(&fread);

    FD_SET(sockfd,&fread);
    max_sd = sockfd;

    for(int i(0) ; i < max_clients ; i++)
    {
        sd = client_socket[i];

        if(sd > 0) FD_SET(sd,&fread);

        if(sd > max_sd) max_sd = sd;
    }

    /* for(map<int,string>::iterator UserSocket = Usernames.begin(); UserSocket!=Usernames.end();UserSocket++)
    {
        sd = (*UserSocket).first;

        if(sd > 0) FD_SET(sd,&fread);

        if(sd > max_sd) max_sd = sd;
    } */

    // int sel = select(max_sd, &fread, &fwrite, &ferror, &tv);

    activity = select(max_sd + 1, &fread, NULL, NULL, &tv);
    
    if(FD_ISSET(sockfd,&fread))
    {
        int new_socket = accept(sockfd,(sockaddr*)&addr,&addrlen);
        if(new_socket < 0)
        {
            // erreur : impossible d'accepter la connection
            wlog("ACCEPT - MERDE!");
            throw new exception();
        }else wlog("ACCEPT - OK!",2);


        for(int i(0); i < max_clients ; i++)
        {
            if(client_socket[i] == 0)
            {
                client_socket[i] = new_socket;
                client_name[i] = "User";
                this->wlog("New client !");
                this->wlog("New client's position : "+to_string(i+1),2);
                break;
            }
        }
        // Usernames[new_socket] = ("User#" + to_string(Usernames.size()));
    }
}

void Server::ops()
{
    int sd, valread;
    // operations on other socket
    for(int i(0) ; i < max_clients ; i++)
    {
        // sd = (*UserSocket).first;
        sd = client_socket[i];
        
        if(FD_ISSET(sd,&fread)) // check socket activity
        {
            buffer = new char[bufferSize];
            valread = recv(sd,buffer,bufferSize,0);
            if(valread == 0)
            {
                if(client_name[i]!="UPDATER")
                { 
                    cout<<endl<<client_name[i]<<" s'est deconnecter."<<endl<<endl;
                    this->wlog("LOGOUT client #"+to_string(i+1)+" || name : "+client_name[i],2);
                }
                else
                {
                    this->wlog("LOGOUT UPDATER");
                }
                shutdown(sd,2); // closing connection
                client_socket[i] = 0; // clean for reuse
                client_name[i] = "";
            }
            else if(valread > 0)
            {
                buffer[valread]='\0';
                string msg = buffer;
                this->wlog("Incoming : "+msg,2);
                if(msg.substr(0,10) == "//version=") // checking client version when asked
                {
                    this->wlog("Receiving client version info");
                    string checkingVersion;
                    checkingVersion = msg.substr(10,msg.size());
                    if(clientVersion == checkingVersion)
                    {
                        this->sendMsg(sd,"//check");
                        this->wlog("The client is up to date",2);
                    }
                    else
                    {
                        this->sendMsg(sd,"//needDl");
                        this->wlog("The client need to update his programme",2);
                        // this->uploadClientVersionUpToDate(sd);
                    }
                }
                else if(msg.substr(0,7) == "//name=") // get the name of the concerned client
                {
                    this->wlog("Receiving client name info");
                    client_name[i]=msg.substr(7,msg.size());
                    if(client_name[i]!="UPDATER")
                    {
                        cout<<endl<<client_name[i]<<" s'est connecter !"<<endl<<endl;
                        this->wlog("LOGINT client #"+to_string(i+1)+" || name : "+client_name[i],2);
                        this->sendMsg(sd,"//check");
                    }
                    else this->wlog("LOGIN UPDATER");
                }
                else if(msg == "//test")
                {
                    
                }
                else if(msg == "//testDownload")
                {
                    this->wlog("A client want to test the download process");
                    
                    string path;
                    ifstream isExiste;
                    do
                    {
                        int val = recv(sd,buffer,bufferSize,0);
                        buffer[val] = '\0';
                        path=buffer;
                        isExiste.open(path.c_str());
                        
                        if(isExiste)
                        {
                            this->sendMsg(sd,"//ready");
                        }
                        else
                        {
                            this->sendMsg(sd,"//retry");
                            isExiste.close();
                        }

                    }while(!isExiste);

                    isExiste.close();

                    delete[] buffer; buffer=0;
                    this->uploadFile(path.c_str(),sd);
                }
                else if(msg == "//updater")
                {
                    this->uploadClientVersionUpToDate(sd);
                }
                else if(msg.substr(0,2) == "//")
                {
                    
                }
                else
                {
                    cout<<client_name[i]<<" -> "<<msg<<endl;
                    this->wlog("Client #"+to_string(i+1)+" "+client_name[i]+" -> "+msg);
                }
            }
            else
            {
                //Erreur
                wlog("C'est la merde...");
                cout<<"C'est la merde..."<<endl;
                throw new exception();
            }
            delete[] buffer;
            buffer=0;
        }
    }
}

void Server::wlog(string msg,int enter)
{   
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

// -------------------------------------------------------- //
                    // ↓ Safe zone ↓ //
                 // touche pas romain //
                  
    ofstream write("./log.txt", ios::app);
    write<<timeinfo->tm_hour<<':'<<timeinfo->tm_min<<':'<<timeinfo->tm_sec<<':'<<msecs_time%1000<<'\t';
    write<<msg;
    for(int i(0);i<enter;i++) write<<endl;
}

void Server::wlog(string msg)
{
    this->wlog(msg,1);
}

void Server::sendMsg(int socket,string msg,bool isLoged)
{
    send(socket,msg.c_str(),msg.size(),0);
    if(isLoged) this->wlog("Message send : "+msg);
}

void Server::sendMsg(int socket,string msg)
{
    this->sendMsg(socket,msg,true);
}

void Server::uploadClientVersionUpToDate(int socket)
{
    string upToDateClientPath("../client/client.exe"),
    bat("../updater/updatingclient.bat");

    this->wlog("Uploading exe");
    this->uploadFile(upToDateClientPath,socket);

    /* this->wlog("Uploading bat");
    this->uploadFile(bat,socket); */

    this->wlog("Sending the correct version number : "+clientVersion);
    this->sendMsg(socket,clientVersion);
}

void Server::uploadFile(string inputPath, int target)
{
    this->wlog("File : "+inputPath);
    this->wlog("client socket : "+to_string(target));
    ifstream input(inputPath.c_str(), ios::binary);
    char buf[bufferSize],data[bigBufferSize];
    int valread;
    
    if(!input)
    {
        this->wlog("Fatal error : cant read the file");
        throw new exception();
    }

    this->wlog("Waiting the //ready...");
    if(recv(target,buf,bufferSize,0) < 0)
    {
        wlog("Failed receiving");
        throw new exception();
    }
    
    this->wlog("//ready recieved");
    
    this->sendMsg(target,"//ready");

    input.seekg(0,ios::end);
    this->wlog("File size : "+to_string(input.tellg()));
    this->sendMsg(target, to_string(input.tellg()));
    input.seekg(0,ios::beg);

    recv(target,buf,bufferSize,0);

    this->wlog("Starting upload...");

    while(input.read(data,bigBufferSize))
    {
        this->sendMsg(target,to_string(input.gcount()),false); // send streamSize
        recv(target,buf,bufferSize,0); // Waiting the client

        send(target,data,bigBufferSize,0); // send the binary data
        recv(target,buf,bufferSize,0); // Waiting the client
    }
    this->sendMsg(target,"//last",false); // Final paquet
    recv(target,buf,bufferSize,0); // Waiting the client

    this->sendMsg(target,to_string(input.gcount()),false); // send streamSize
    recv(target,buf,bufferSize,0); // Waiting the client

    send(target,data,bigBufferSize,0); // send the binary data
    recv(target,buf,bufferSize,0); // Waiting the client

    this->wlog("Upload completed !",2);
}