#include "DatabaseFingerprinter.h"
#include "SocketUtils.h"
#include "NetworkScannerWorker.h"

#include <unistd.h>


DatabaseType DatabaseFingerprinter::MySQL_MariaBD(SocketHandle sock) {
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    if (buffer[4] != 0x0A) {
        return DatabaseType::Unknown;
    }
    std::string version(buffer + 5);

    if (version.find("MariaDB") != std::string::npos) {
        return DatabaseType::MariaDB;
    }
    return DatabaseType::MySQL;
}

DatabaseType DatabaseFingerprinter::Redis(SocketHandle sock) {
    const char request[] = "*1\r\n$4\r\nPING\r\n";
    if (send(sock, request, sizeof(request) - 1, 0) <= 0) {
        return DatabaseType::Unknown;
    }

    char buffer[1024];

    int received = recv(sock, buffer, sizeof(buffer), 0);

    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    std::string response(buffer, received);

    if (response.rfind("+PONG", 0) == 0) {
        return DatabaseType::Redis;
    }

    if (response.rfind("-NOAUTH", 0) == 0) {
        return DatabaseType::Redis;
    }
    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::PostgreSQL(SocketHandle sock) {
    char msg[128];
    int pos = 4;
    uint32_t protocol = htonl(196608);
    memcpy(msg + pos, &protocol, 4);
    pos += 4;
    memcpy(msg + pos, "user", sizeof("user"));
    pos += sizeof("user");
    memcpy(msg + pos, "test", sizeof("test"));
    pos += sizeof("test");
    memcpy(msg + pos, "database", sizeof("database"));
    pos += sizeof("database");
    memcpy(msg + pos, "test", sizeof("test"));
    pos += sizeof("test");
    msg[pos++] = '\0';
    uint32_t len = htonl(pos);
    memcpy(msg, &len, 4);

    if (send(sock, msg, pos, 0) <= 0) {
        return DatabaseType::Unknown;
    }

    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    std::cout << "received = " << received << '\n';

    if (received > 0)
        std::cout << "first byte = " << buffer[0] << '\n';
    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    if (buffer[0] == 'R' || buffer[0] == 'E') {
        return DatabaseType::PostgreSQL;
    }
    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::MSSQL(SocketHandle sock) {
    const unsigned char request[] =
    {
        0x12, 0x01, 0x00, 0x34,
        0x00, 0x00, 0x01, 0x00,

        0x00, 0x00, 0x15, 0x00, 0x06,
        0x01, 0x00, 0x1B, 0x00, 0x01,
        0x02, 0x00, 0x1C, 0x00, 0x0C,
        0x03, 0x00, 0x28, 0x00, 0x04,
        0xFF,
        0x08, 0x00, 0x01,
        0x55, 0x00, 0x00, 0x00,
        0x4D, 0x53, 0x53, 0x51,
        0x4C, 0x53, 0x65, 0x72,
        0x76, 0x65, 0x72, 0x00,
        0x80, 0x19, 0x00, 0x00
    };


    if (send(sock, reinterpret_cast<const char *>(request), sizeof(request), 0) <= 0) {
        return DatabaseType::Unknown;
    }
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    std::cout << "received = " << received << std::endl;

    if (received == -1) {
        std::cout << "WSA error = " << WSAGetLastError() << '\n';
    }

    for (int i = 0; i < received; ++i) {
        printf("%02X", static_cast<unsigned char>(buffer[i]));
    }
    printf("\n");


    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    if (static_cast<unsigned char>(buffer[0]) == 0x04) {
        return DatabaseType::MSSQL;
    }
    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::Oracle(SocketHandle sock) {
    const std::string connectData = "(CONNECT_DATA=(COMMAND=version))";

    std::vector<unsigned char> request;

    auto addUInt16 = [&](uint16_t value) {
        request.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
        request.push_back(static_cast<unsigned char>(value & 0xFF));
    };

    auto addUInt32 = [&](uint32_t value) {
        request.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
        request.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
        request.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
        request.push_back(static_cast<unsigned char>(value & 0xFF));
    };

    addUInt16(static_cast<uint16_t>(connectData.size() + 34));

    addUInt16(0);
    request.push_back(0x01);
    request.push_back(0x00);
    addUInt16(0);
    addUInt16(308);
    addUInt16(300);
    addUInt16(0);
    addUInt16(2048);
    addUInt16(32767);
    addUInt16(20376);
    addUInt16(0);
    addUInt16(1);

    addUInt16(static_cast<uint16_t>(connectData.size()));

    addUInt16(34);
    addUInt32(0);
    request.push_back(0x01);
    request.push_back(0x01);
    request.insert(request.end(), connectData.begin(), connectData.end());

    int sent = send(sock, reinterpret_cast<const char *>(request.data()), static_cast<int>(request.size()), 0);
    if (sent != static_cast<int>(request.size())) {
        return DatabaseType::Unknown;
    }
    unsigned char buffer[4096];
    int received = recv(sock, reinterpret_cast<char *>(buffer), sizeof(buffer), 0);
    if (received <= 0) {
        return DatabaseType::Unknown;
    }
    if (received < 8) {
        return DatabaseType::Unknown;
    }
    const std::string response(
        reinterpret_cast<const char *>(buffer),
        received
    );

    if (response.find("(VSNNUM=") != std::string::npos) {
        return DatabaseType::Oracle;
    }

    if (response.find("(ERR=") != std::string::npos) {
        return DatabaseType::Oracle;
    }
    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::DB2(SocketHandle sock) {
    const unsigned char request[] =
    {
        0x00, 0x32, 0xD0, 0x01,
        0x00, 0x01, 0x00, 0x2C,
        0x10, 0x41, 0x00, 0x04,
        0x11, 0x5E, 0x00, 0x04,
        0x11, 0x6D, 0x00, 0x04,
        0x11, 0x5A, 0x00, 0x18,
        0x14, 0x04, 0x14, 0x03,
        0x00, 0x07, 0x24, 0x07,
        0x00, 0x08, 0x24, 0x0F,
        0x00, 0x08, 0x14, 0x40,
        0x00, 0x08, 0x14, 0x74,
        0x00, 0x08, 0x00, 0x04,
        0x11, 0x47
    };

    int sent = send(sock, reinterpret_cast<const char *>(request), sizeof(request), 0);

    if (sent != sizeof(request)) {
        return DatabaseType::Unknown;
    }
    unsigned char buffer[4096];
    int received = recv(sock, reinterpret_cast<char *>(buffer), sizeof(buffer), 0);

    if (received <= 0) {
        return DatabaseType::Unknown;
    }

    const std::string response(reinterpret_cast<const char *>(buffer), received);

    if (response.find("DB2") != std::string::npos || response.find("SQL") != std::string::npos) {
        return DatabaseType::IBM_DB2;
    }

    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::MongoDB(SocketHandle sock) {
    static std::atomic<int32_t> nextRequestId{1};

    auto appendInt32 = [](std::vector<unsigned char> &data, int32_t value) {
        data.push_back(static_cast<unsigned char>(value & 0xFF));
        data.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
        data.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
        data.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
    };

    auto appendCString = [](std::vector<unsigned char> &data, const std::string &value) {
        data.insert(data.end(), value.begin(), value.end());
        data.push_back(0x00);
    };

    auto appendBsonInt32 = [&](std::vector<unsigned char> &data, const std::string &name, int32_t value) {
        data.push_back(0x10);
        appendCString(data, name);
        appendInt32(data, value);
    };

    auto appendBsonString = [&](std::vector<unsigned char> &data, const std::string &name, const std::string &value) {
        data.push_back(0x02);
        appendCString(data, name);

        appendInt32(data, static_cast<int32_t>(value.size() + 1));
        data.insert(data.end(), value.begin(), value.end());
        data.push_back(0x00);
    };

    std::vector<unsigned char> bson;

    appendInt32(bson, 0);
    appendBsonInt32(bson, "hello", 1);

    appendBsonString(bson, "$db", "admin");
    bson.push_back(0x00);

    const int32_t bsonLength = static_cast<int32_t>(bson.size());

    bson[0] = static_cast<unsigned char>(bsonLength & 0xFF);
    bson[1] = static_cast<unsigned char>((bsonLength >> 8) & 0xFF);
    bson[2] = static_cast<unsigned char>((bsonLength >> 16) & 0xFF);
    bson[3] = static_cast<unsigned char>((bsonLength >> 24) & 0xFF);

    const int32_t requestId = nextRequestId.fetch_add(1);
    std::vector<unsigned char> request;

    const int32_t messageLength = 16 + 4 + 1 + static_cast<int32_t>(bson.size());

    appendInt32(request, messageLength);
    appendInt32(request, requestId);
    appendInt32(request, 0);
    appendInt32(request, 2013);

    appendInt32(request, 0);

    request.push_back(0x00);
    request.insert( request.end(), bson.begin(), bson.end());
    int sent = send(sock, reinterpret_cast<const char *>(request.data()), static_cast<int>(request.size()), 0);
    if (sent != static_cast<int>(request.size())) {
        return DatabaseType::Unknown;
    }

    std::vector<unsigned char> header(16);

    size_t headerReceived = 0;

    while (headerReceived < header.size()) {
        int received = recv(sock, reinterpret_cast<char *>(header.data() + headerReceived), static_cast<int>(header.size() - headerReceived),0);

        if (received <= 0) {
            return DatabaseType::Unknown;
        }

        headerReceived += static_cast<size_t>(received);
    }

    auto readInt32 = [](const unsigned char *data) -> int32_t {
        return static_cast<int32_t>(data[0]) | (static_cast<int32_t>(data[1]) << 8) |(static_cast<int32_t>(data[2]) << 16) | (static_cast<int32_t>(data[3]) << 24);
    };
    const int32_t responseLength =readInt32(header.data());
    const int32_t responseTo = readInt32(header.data() + 8);
    const int32_t opCode = readInt32(header.data() + 12);
    if (responseLength < 21 || responseLength > 48 * 1024 * 1024) {
        return DatabaseType::Unknown;
    }

    if (responseTo != requestId) {
        return DatabaseType::Unknown;
    }

    if (opCode != 2013) {
        return DatabaseType::Unknown;
    }

    const size_t bodySize = static_cast<size_t>(responseLength - 16);
    std::vector<unsigned char> body(bodySize);

    size_t bodyReceived = 0;

    while (bodyReceived < bodySize) {
        int received = recv(sock, reinterpret_cast<char *>(body.data() + bodyReceived),static_cast<int>(bodySize - bodyReceived),0);
        if (received <= 0) {
            return DatabaseType::Unknown;
        }

        bodyReceived += static_cast<size_t>(received);
    }

    if (body.size() < 5) {
        return DatabaseType::Unknown;
    }

    const unsigned char *bsonResponse = nullptr;
    size_t bsonResponseSize = 0;

    size_t offset = 4;

    while (offset < body.size()) {
        unsigned char sectionKind = body[offset];

        if (sectionKind == 0) {
            offset++;
            if (offset + 4 > body.size()) {
                return DatabaseType::Unknown;
            }
            int32_t documentLength = readInt32(body.data() + offset);

            if (documentLength < 5 ||offset + static_cast<size_t>(documentLength) >body.size()) {
                return DatabaseType::Unknown;
            }

            bsonResponse = body.data() + offset;
            bsonResponseSize = static_cast<size_t>(documentLength);

            break;
        }

        return DatabaseType::Unknown;
    }

    if (bsonResponse == nullptr) {
        return DatabaseType::Unknown;
    }

    const std::string response(
        reinterpret_cast<const char *>(bsonResponse),
        bsonResponseSize
    );

    if (response.find("isWritablePrimary") != std::string::npos && response.find("maxWireVersion") != std::string::npos) {
        return DatabaseType::MongoDB;
    }

    if (response.find("maxWireVersion") != std::string::npos && response.find("minWireVersion") != std::string::npos) {
        return DatabaseType::MongoDB;
    }

    if (response.find("isdbgrid") != std::string::npos) {
        return DatabaseType::MongoDB;
    }

    return DatabaseType::Unknown;
}


DatabaseType DatabaseFingerprinter::identify(SocketHandle sock, quint16 port) {
    switch (port) {
        case 3306:
            return MySQL_MariaBD(sock);
        case 6379:
            return Redis(sock);
        case 5432:
            return PostgreSQL(sock);
        case 1433:
            return MSSQL(sock);
        case 1521:
            return Oracle(sock);
        case 50000:
            return DB2(sock);
        case 27017:
            return MongoDB(sock);
        default:
            return DatabaseType::Unknown;
    }
}

DatabaseType DatabaseFingerprinter::identifyUnknown(const QString &ip, quint16 port) {
    DatabaseType db;
    SocketHandle sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = MySQL_MariaBD(sock);
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = PostgreSQL(sock);
        std::cout << "db = " << static_cast<int>(db) << '\n';
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = MSSQL(sock);
        std::cout << "db = " << static_cast<int>(db) << '\n';
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = Redis(sock);
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = Oracle(sock);
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = DB2(sock);
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }
    sock = SocketUtils::connectToHost(ip, port);
    if (sock != InvalidSocket) {
        db = MongoDB(sock);
        SocketUtils::closeSocket(sock);
        if (db != DatabaseType::Unknown)
            return db;
    }

    return DatabaseType::Unknown;
}
