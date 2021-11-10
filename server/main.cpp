// Ne pas oublier d'inclurer "-lwsock32" après "server.exe" dans la commande de compilation.

#include "server.hpp"
#include "windows.h"

using namespace std;

int main()
{
    system("cls");
    Server srv;

    srv.boot();
    srv.run();

    return 0;
}