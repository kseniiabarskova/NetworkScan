#ifndef QTSCANNER_NETWORK_1_SOCKETUTILS_H
#define QTSCANNER_NETWORK_1_SOCKETUTILS_H
#include <iostream>
#include <QString>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>

using SocketHandle = SOCKET;
constexpr SocketHandle InvalidSocket = INVALID_SOCKET;

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using SocketHandle = int;
constexpr SocketHandle InvalidSocket = -1;


#endif
namespace SocketUtils {
    bool initialize();

    void cleanup();

    SocketHandle createSocket();

    void closeSocket(SocketHandle sock);

    int lastError();

    bool setNonBlocked(SocketHandle sock);
    bool waitForConnection(SocketHandle sock);

    bool connectionInProgress();

    bool setBlocking(SocketHandle sock);

    bool setRecvTimeout(SocketHandle sock);

    SocketHandle connectToHost(const QString& ip, quint16 port);

}

#endif
