#include "SocketUtils.h"

#ifdef _WIN32

bool SocketUtils::initialize() {
    WSAData wsData;
    int erStat = WSAStartup(MAKEWORD(2,2), &wsData);
    if (erStat != 0) {
        std::cout << "Error WinSock version initializaion";
        return false;
    }
    std::cout << "WinSock initialization is OK" << std::endl;
    return true;
}

void SocketUtils::cleanup() {
    WSACleanup();
}

SocketHandle SocketUtils::createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void SocketUtils::closeSocket(SocketHandle sock) {
    closesocket(sock);
}


int SocketUtils::lastError()
{
    return WSAGetLastError();
}

#else
bool SocketUtils::initialize() {
    return true;
}
void SocketUtils::cleanup()
{
}

SocketHandle SocketUtils::createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}


void SocketUtils::closeSocket(SocketHandle sock)
{
    close(sock);
}


int SocketUtils::lastError()
{
    return errno;
}


#endif



