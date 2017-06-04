#include "socketworker.h"
#include "tcp_client.h"

SocketWorker::SocketWorker(int s) {
    _sockId = s;
}

void SocketWorker::receivingMessages() {
    // get parsing info
    char message[256];
    int n = -1;
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
    }
    // get weights info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
    }
    // load imnage info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
    }
    // start predict
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
    }
    // predict info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
    }
    // get image
    emit finished("");
}
