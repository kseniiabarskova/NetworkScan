#include "NetworkScannerWorker.h"
#include <vector>
#include <iostream>
#include "SocketUtils.h"
#include "DatabaseFingerprinter.h"

NetworkScannerWorker::NetworkScannerWorker(const std::vector<QString> &_ipList, const std::vector<quint16> &_ports):ipList(_ipList), ports(_ports){
    bool init = SocketUtils::initialize();
    if (!init) {
        throw std::runtime_error("Failed to initialize sockets");
    }

}

NetworkScannerWorker::~NetworkScannerWorker()
{
    SocketUtils::cleanup();
}




SocketHandle NetworkScannerWorker::checkPort(const QString& ip, quint16 port) {

    SocketHandle sock = SocketUtils::createSocket();
    if (sock == InvalidSocket)
    {
        std::cerr << "Failed to create socket\n";
        return InvalidSocket;
    }

    sockaddr_in servInfo{};
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(port);

    std::string ipStr = ip.toStdString();
    int erStat = inet_pton(AF_INET,ipStr.c_str(), &servInfo.sin_addr);
    if (erStat != 1) {
        std::cout << "Error in IP translation to special numeric format" << std::endl;
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }

    int result = connect(sock, reinterpret_cast<sockaddr*>(&servInfo), sizeof(servInfo));
    if (result != 0) {
        SocketUtils::closeSocket(sock);
        return InvalidSocket;
    }
    return sock;
}



void NetworkScannerWorker::scan() {
    for (const auto& ip : ipList) {
        for (const auto& port : ports) {
            SocketHandle sock = checkPort(ip, port);
            if (sock == InvalidSocket) {
                continue;
            }
            DatabaseType db = DatabaseFingerprinter::identify(sock, port);
            switch (db)
            {
                case DatabaseType::MySQL:
                    std::cout << ip.toStdString() << ":" << port<< " MySQL\n";
                    break;

                case DatabaseType::MariaDB:
                    std::cout << ip.toStdString()<< ":" << port<< " MariaDB\n";
                    break;

                case DatabaseType::Redis:
                    std::cout << ip.toStdString()
                              << ":" << port
                              << " Redis\n";
                    break;

                default:
                    std::cout << ip.toStdString()
                              << ":" << port
                              << " Unknown\n";
                    break;
            }

            SocketUtils::closeSocket(sock);
        }
    }
}










