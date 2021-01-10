#include "networkadapter.h"

NetworkAdapter::NetworkAdapter(): _threadQuit(false), _clientSocket(nullptr),
    _port(0)
{
    _tcpServer = new QTcpServer();
    connect(_tcpServer, &QTcpServer::newConnection,
            this, &NetworkAdapter::_onNewConnection);

    _mux = DataMultiplexer::GetP();
}

NetworkAdapter::~NetworkAdapter()
{
    //  Pass kill signal to the thread and wait for it to terminate
    _threadQuit = true;
    wait();
    StopListening();
}

/**
 * @brief Network port setted
 * @param port
 */
void NetworkAdapter::SetNetPort(uint16_t port)
{
    _port = port;
}

/**
 * @brief Network port getter
 * @return
 */
uint16_t NetworkAdapter::GetPort()
{
    return _port;
}

/**
 * @brief Start listening on network port
 *      Starts up TCP server on a port
 */
bool NetworkAdapter::StartListening()
{
    if (!_tcpServer->isListening())
    {
        //  External process use this flag to control thread, avoid
        //  forcing its value here
        //_threadQuit = false;

        if (_port == 0)
        {
            return false;
            emit logLine("Port cannot be 0");
        }

        _tcpServer->listen(QHostAddress::Any, _port);
        _tcpServer->resumeAccepting();
        emit logLine("Waiting for new connection on port "+QString::number(_port));
    }

    return true;
}

/**
 * @brief Stop listening on network port
 *      Stop TCP server and tear down a client connection if it exists
 */
void NetworkAdapter::StopListening()
{
    emit logLine("Disconnecting client");

    if (_clientSocket != nullptr)
        if (_clientSocket->isOpen())
        {
            disconnect(_clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            disconnect(_clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                       this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
            _clientSocket->close();
            delete _clientSocket;
            _clientSocket = nullptr;
        }

    emit logLine("Stopping network thread");
    if (_tcpServer->isListening())
        _tcpServer->close();
}

/**
 * @brief [Slot] Handler for new TCP connection
 *      Called by TCP server whenever a new client is asking to be connected
 */
void NetworkAdapter::_onNewConnection()
{
    _clientSocket = _tcpServer->nextPendingConnection();
    _tcpServer->pauseAccepting();
    emit logLine("Connection established, reading data");
    //start();

    connect(_clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(_clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

/**
 * @brief [Slot] Handler for state change in sockets
 *      Used to handle a client disconnecting from the server. Functions log
 *      disconnect, delete client object and reset the pointer. Afterwards,
 *      the server is enabled to receive new clients.
 * @param socketState socket state after a change
 */
void NetworkAdapter::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        emit logLine("Client disconnected, waiting for new connection");
        _clientSocket->deleteLater();
        _clientSocket = nullptr;
        _tcpServer->resumeAccepting();
    }
}

/**
 * @brief [Slot] Handler for new incoming data on the socket
 *      Called by the client socket when there's new data available
 */
void NetworkAdapter::onReadyRead()
{
    QByteArray responseData = _clientSocket->readAll();
    const QString response = QString::fromUtf8(responseData);

    _mux->ReceiveData(response);
}
