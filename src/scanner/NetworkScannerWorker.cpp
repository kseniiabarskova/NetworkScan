#include "NetworkScannerWorker.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include "SocketUtils.h"
#include "DatabaseFingerprinter.h"

NetworkScannerWorker::NetworkScannerWorker(std::vector<QString> _ipList, std::vector<quint16> _ports, bool _rangeMode) : ipList(_ipList), ports(_ports), rangeMode(_rangeMode) {
    bool init = SocketUtils::initialize();
    if (!init) {
        throw std::runtime_error("Failed to initialize sockets");
    }
    currentIp = 0;
    currentPort = 0;
}

NetworkScannerWorker::~NetworkScannerWorker() {
    SocketUtils::cleanup();
}
void NetworkScannerWorker::requestStop()
{
    stopReq.store(true);
    stoppedByUser.store(true);
}


bool NetworkScannerWorker::getNextTask(ScanTask &task) {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentIp >= ipList.size()) {
        return false;
    }
    task.ip = ipList[currentIp];
    task.port = ports[currentPort];

    ++currentPort;
    if (currentPort >= ports.size()) {
        currentPort = 0;
        ++currentIp;
    }
    return true;

}

void NetworkScannerWorker::workerThread() {
    ScanTask task;
    while (!stopReq && getNextTask(task)) {
        SocketHandle sock = SocketUtils::connectToHost(task.ip, task.port);
        if (sock != InvalidSocket) {
        auto start = std::chrono::steady_clock::now();

            DatabaseType db;
            if (rangeMode) {
                SocketUtils::closeSocket(sock);
                db = DatabaseFingerprinter::identifyUnknown(task.ip, task.port);
            }else {
                db = DatabaseFingerprinter::identify(sock, task.port);
                SocketUtils::closeSocket(sock);

            }
        auto end = std::chrono::steady_clock::now();
        double responseTime = std::chrono::duration<double, std::milli>(end - start).count();

            if (db != DatabaseType::Unknown) {
                emit databaseFound(task.ip, task.port, db, responseTime);
            }
        }
        size_t done = ++completedTask;
        int perc = static_cast<int>(done*100/totalTasks);
        std::cout << "Progress =" << perc << std::endl;
        if (perc != lastPercent.exchange(perc))
        {
            emit progressChanged(perc);
        }
    }
}

bool NetworkScannerWorker::wasStopped() const
{
    return stoppedByUser.load();
}
void NetworkScannerWorker::scan() {
    stopReq.store(false);
    stoppedByUser.store(false);
    currentIp = 0;
    currentPort = 0;
    unsigned int threadCount = std::thread::hardware_concurrency();

    if (threadCount == 0)
        threadCount = 8;
    std::vector<std::thread> threads;
    std::cout << "Threads = " << threadCount << '\n';
    threads.reserve(threadCount);
    completedTask = 0;
    totalTasks = ipList.size() * ports.size();
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(&NetworkScannerWorker::workerThread, this);

    }

    for (auto& thread : threads) {
        thread.join();
    }
    emit finished();

}


