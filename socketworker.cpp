#include "socketworker.h"
#include "tcp_client.h"

SocketWorker::SocketWorker(int s) {
    _sockId = s;
}

void SocketWorker::receivingMessages() {
    char message[256];
    int n = -1;
     // get parsing info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
        return;
    }
    // get weights info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
        return;
    }
    // load imnage info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
        return;
    }
    // start predict
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
        return;
    }
    // predict info
    n = readMessage(_sockId, message);
    if (n != -1) {
        emit messageReceived(QString(message));
    }
    else {
        emit messageReceived("");
        return;
    }
    // get image
    emit finished("");
}
