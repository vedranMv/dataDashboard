#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include "helperObjects/dataMultiplexer/datamultiplexer.h"


class NetworkAdapter : public QThread
{
    Q_OBJECT
public:
    NetworkAdapter();
    ~NetworkAdapter();

    void SetNetPort(uint16_t port);
    uint16_t GetPort();
    bool StartListening();
    void StopListening();

signals:
    void logLine(const QString &s);

private slots:
        void _onNewConnection();
        void onSocketStateChanged(QAbstractSocket::SocketState socketState);
        void onReadyRead();

private:

    bool _threadQuit;
    QTcpServer *_tcpServer;
    QTcpSocket *_clientSocket;

    uint16_t _port;
    DataMultiplexer* _mux;
};

#endif // NETWORKADAPTER_H
