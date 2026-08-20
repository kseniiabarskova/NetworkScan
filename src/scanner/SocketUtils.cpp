#include "SocketUtils.h"

#ifdef _WIN32

bool SocketUtils::initialize() {
    WSAData wsData;
    int erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
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


int SocketUtils::lastError() {
    return WSAGetLastError();
}

bool SocketUtils::setNonBlocked(SocketHandle sock) {
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool SocketUtils::waitForConnection(SocketHandle sock) {
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);
    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    int result = select(0, nullptr, &writeSet, nullptr, &tv);
    if (result == 0) {
        std::cout << "timeout" << std::endl;
        return false;
    }
    if (result < 0) {
        std::cout << "error" << std::endl;
        return false;
    }
    int err = 0;
    int len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&err), &len) == SOCKET_ERROR) {
        std::cout << "getsockopt() failed\n";
        return false;
    }
    std::cout << "SO_ERROR = " << err << std::endl;
    if (err != 0) {
        std::cout << "Connection failed." << std::endl;
        return false;
    }
    return true;
}

bool SocketUtils::connectionInProgress() {
    return WSAGetLastError() == WSAEWOULDBLOCK;
}

bool SocketUtils::setBlocking(SocketHandle sock) {
    u_long mode = 0;
    if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool SocketUtils::setRecvTimeout(SocketHandle sock) {
    DWORD timeout = 2000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0;
}

SocketHandle SocketUtils::connectToHost(const QString &ip, quint16 port) {
    SocketHandle sock = SocketUtils::createSocket();
    if (sock == InvalidSocket) {
        std::cerr << "Failed to create socket\n";
        return InvalidSocket;
    }

    sockaddr_in servInfo{};
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(port);

    std::string ipStr = ip.toStdString();
    int erStat = inet_pton(AF_INET, ipStr.c_str(), &servInfo.sin_addr);
    if (erStat != 1) {
        std::cout << "Error in IP translation to special numeric format" << std::endl;
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }

    if (!SocketUtils::setNonBlocked(sock)) {
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "connect " << ip.toStdString() << ":" << port << '\n';
    int result = ::connect(sock, reinterpret_cast<sockaddr *>(&servInfo), sizeof(servInfo));
    std::cout << "after connect\n";
    if (result == 0) {
        std::cout << "connected immediately\n";
        SocketUtils::setBlocking(sock);
        SocketUtils::setRecvTimeout(sock);
        return sock;
    }
    std::cout << "connectionInProgress\n";
    if (!SocketUtils::connectionInProgress()) {
        std::cout << "not in progress\n";
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "waitForConnection\n";
    if (!SocketUtils::waitForConnection(sock)) {
        std::cout << "wait failed\n";
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "setBlocking\n";
    if (!SocketUtils::setBlocking(sock)) {
        SocketUtils::closeSocket(sock);
        std::cout << "setBlocking failed\n";
        return InvalidSocket;
    }

    if (!SocketUtils::setRecvTimeout(sock)) {
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "connected OK\n";
    std::cout << "return socket\n";
    return sock;
}

#else
bool SocketUtils::initialize() {
    return true;
}
void SocketUtils::cleanup() {
}

SocketHandle SocketUtils::createSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}


void SocketUtils::closeSocket(SocketHandle sock) {
    close(sock);
}


int SocketUtils::lastError() {
    return errno;
}

void SocketUtils::setNonBlocked(SocketHandle sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}
bool SocketUtils::waitForConnection(SocketHandle sock) {
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);
    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    int result = select(sock + 1, nullptr, &writeSet, nullptr, &tv);
    if (result == 0) {
        std::cout << "timeout" << std::endl;
        return false;
    }
    if (result < 0) {
        std::cout << "error" << std::endl;
        return false;
    }
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) == -1) {
        std::cout << "error getsockopt()" << std::endl;
        return false;
    }
    if (err != 0) {
        std::cout << "Connection failed." << std::endl;
        return false;
    }
    return true;
}
bool SocketUtils::connectionInProgress() {
    return errno == EINPROGRESS;
}

bool SocketUtils::setBlocking(SocketHandle sock) {
    int flags = fcntl(sock, F_GETFL, 0);

    if (flags == -1) {
        return false;
    }

    flags &= ~O_NONBLOCK;

    if (fcntl(sock, F_SETFL, flags) == -1) {
        return false;
    }

    return true;
}
bool SocketUtils::setRecvTimeout(SocketHandle sock) {
    timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
}

SocketHandle SocketUtils::connectToHost(const QString &ip, quint16 port) {
    SocketHandle sock = SocketUtils::createSocket();
    if (sock == InvalidSocket) {
        std::cerr << "Failed to create socket\n";
        return InvalidSocket;
    }

    sockaddr_in servInfo{};
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(port);

    std::string ipStr = ip.toStdString();
    int erStat = inet_pton(AF_INET, ipStr.c_str(), &servInfo.sin_addr);
    if (erStat != 1) {
        std::cout << "Error in IP translation to special numeric format" << std::endl;
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }

    if (!SocketUtils::setNonBlocked(sock)) {
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "connect " << ip.toStdString() << ":" << port << '\n';
    int result = ::connect(sock, reinterpret_cast<sockaddr *>(&servInfo), sizeof(servInfo));
    std::cout << "after connect\n";
    if (result == 0) {
        std::cout << "connected immediately\n";
        SocketUtils::setBlocking(sock);
        SocketUtils::setRecvTimeout(sock);
        return sock;
    }
    std::cout << "connectionInProgress\n";
    if (!SocketUtils::connectionInProgress()) {
        std::cout << "not in progress\n";
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "waitForConnection\n";
    if (!SocketUtils::waitForConnection(sock)) {
        std::cout << "wait failed\n";
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "setBlocking\n";
    if (!SocketUtils::setBlocking(sock)) {
        SocketUtils::closeSocket(sock);
        std::cout << "setBlocking failed\n";
        return InvalidSocket;
    }

    if (!SocketUtils::setRecvTimeout(sock)) {
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    std::cout << "connected OK\n";
    std::cout << "return socket\n";
    return sock;
}
#endif
