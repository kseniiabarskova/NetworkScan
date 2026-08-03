#include "DatabaseFingerprinter.h"
#include "SocketUtils.h"

#include <unistd.h>
DatabaseType DatabaseFingerprinter::MySQL_MariaBD(SocketHandle sock) {
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0 ) {
        return DatabaseType::Unknown;
    }
    if (buffer[4] != 0x0A)
    {
        return DatabaseType::Unknown;
    }
    std::string version(buffer + 5);

    if (version.find("MariaDB") != std::string::npos)
    {
        return DatabaseType::MariaDB;
    }
    return DatabaseType::MySQL;
}

DatabaseType DatabaseFingerprinter::Redis(SocketHandle sock)
{
    const char request[] = "*1\r\n$4\r\nPING\r\n";
    if (send(sock, request, sizeof(request) - 1, 0) <= 0)
    {
        return DatabaseType::Unknown;
    }

    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    std::string response(buffer, received);

    if (response.rfind("+PONG", 0) == 0)
    {
        return DatabaseType::Redis;
    }

    if (response.rfind("-NOAUTH", 0) == 0)
    {
        return DatabaseType::Redis;
    }

    return DatabaseType::Unknown;
}

/*DatabaseType DatabaseFingerprinter::PostgreSQL(SocketHandle sock) {
    char message[128];
    int version = 196608;
    std::memcpy(&message[4], &version, sizeof(version));
    std::string user("user");
    std::memcpy(&message[8], &user, sizeof(user));
    std::string base("postgres");
    std::memcpy(&message[8+sizeof(user)], &base, sizeof(base));
    std::string data("database");
    std::memcpy(&message[8+sizeof(user) + sizeof(base)], &data, sizeof(data));
    if (send(sock, message, sizeof(message) - 1, 0) <= 0) {
        return DatabaseType::Unknown;
    }

    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    if (buffer[0] == 'R' || buffer[0] == 'E') {
        return DatabaseType::PostgreSQL;
    }
    return DatabaseType::Unknown;
*/

DatabaseType DatabaseFingerprinter::identify(SocketHandle sock, quint16 port) {
    switch (port) {
        case 3306:
            return MySQL_MariaBD(sock);
        case 6379:
            return Redis(sock);
            default:
            return DatabaseType::Unknown;
    }

}
