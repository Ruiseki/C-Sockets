// Ne pas oublier d'inclurer "-lwsock32" après "client.exe" dans la commande de compilation.

#include <iostream>
#include <string>
#include <winsock2.h>
#include <fstream>
#include "windows.h"
#include "client.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    string serverAddr, port;
    
    if(argc > 1)
    {
        if(argc == 1) port = argv[1];
        else
        {
            port = argv[1];
            serverAddr = argv[2];
        }
    }
    else
    {
        serverAddr = "93.16.231";
        port = "55000";
    }

    Client clientapp(stoi(port), serverAddr);

    int selection(1);
    string command;
    while(selection!=0)
    {
        system("cls");
        cout<<"Current pseudo : "<<clientapp.getPseudo()<<endl;
        cout<<"Current port : "<<clientapp.getPort()<<endl;
        cout<<"1: Change pseudo"<<endl<<
        "2: connect to the server"<<endl<<
        "3: Change port"<<endl<<
        "8: Manual update"<<endl<<
        "9: Test download"<<endl<<
        "0: shutdown client"<<endl;
        cin>>selection;
        switch(selection)
        {
            case 0:
                break;

            case 1:
                clientapp.changePseudo();
                break;

            case 2:
                clientapp.run();
                break;

            case 3:
                clientapp.changePort();
                break;
                
            case 8:
                command = "updater.exe " + port + " " + serverAddr;
                system(command.c_str());
                return 0;
                break;

            case 9:
                clientapp.testDownload();
                break;

            default:
                system("cls");
                break;
        }
    }
    system("cls");
    return 0;
}