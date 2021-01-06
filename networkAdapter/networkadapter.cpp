#include "networkadapter.h"

NetworkAdapter::NetworkAdapter()
{
    _port = 0;
    _threadQuit = false;

    _tcpServer = new QTcpServer();
    connect(_tcpServer, &QTcpServer::newConnection,
            this, &NetworkAdapter::_onNewConnection);
}

NetworkAdapter::~NetworkAdapter()
{
    //  Pass kill signal to the thread and wait for it to terminate
    _threadQuit = true;
    wait();
    StopListening();
}

void NetworkAdapter::SetNetPort(uint16_t port)
{
    _port = port;
}
uint16_t NetworkAdapter::GetPort()
{
    return _port;
}

void NetworkAdapter::StartListening()
{
    if (!_tcpServer->isListening())
    {
        //  External process use this flag to control thread, avoid
        //  forcing its value here
        //_threadQuit = false;


        if (_port == 0)
            return;

        _tcpServer->listen(QHostAddress::Any, _port);
        _tcpServer->resumeAccepting();
        emit logLine("Waiting for new connection on port "+QString::number(_port));
    }
}

void NetworkAdapter::StopListening()
{
    emit logLine("Disconnecting client");
    clientSocket->close();
    emit logLine("Stopping network thread");
    _tcpServer->close();
}

void NetworkAdapter::_onNewConnection()
{
    clientSocket = _tcpServer->nextPendingConnection();
    _tcpServer->pauseAccepting();
    emit logLine("Connection established, reading data");
    //start();

    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

void NetworkAdapter::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        emit logLine("Client disconnected, waiting for new connection");
        clientSocket->deleteLater();
        clientSocket = nullptr;
        _tcpServer->resumeAccepting();
    }
}

void NetworkAdapter::onReadyRead()
{
    QByteArray responseData = clientSocket->readAll();
    const QString response = QString::fromUtf8(responseData);

    _mux->ReceiveSerialData(response);
}


void NetworkAdapter::RegisterMux(DataMultiplexer* mux)
{
    _mux = mux;
}
