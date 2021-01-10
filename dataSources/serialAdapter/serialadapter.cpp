#include "serialadapter.h"

#include <QSerialPort>
#include <QTime>

SerialAdapter::SerialAdapter(QObject *parent) :
    QThread(parent)
{
    _mux = DataMultiplexer::GetP();
}

SerialAdapter::~SerialAdapter()
{
    _threadQuit = true;
    wait();
}

/**
 * @brief Open serial port
 */
void SerialAdapter::StartListening()
{
    //  Check if the thread is already running, if not start it
    if (!isRunning())
    {
        _threadQuit = false;
        start();
    }
}

/**
 * @brief Close serial port
 */
void SerialAdapter::StopListening()
{
    _threadQuit = true;
}

/**
 * @brief Update serial port parameters
 * @param portName
 * @param portBaud
 */
void SerialAdapter::UpdatePort(const QString &portName, const QString &portBaud)
{
    if ((portName == _portName) && (portBaud == _portBaud))
        _portUpdated = false;
    else
    {
        _portUpdated = true;
        _portName = portName;
        _portBaud = portBaud;
    }
}

/**
 * @brief Thread to read and publish serial data, capable of dynamically
 *  updating port parameters
 */
void SerialAdapter::run()
{
    QSerialPort serial;

    emit logLine("Serial thread started");

    if (_portName.isEmpty())
    {
        emit logLine(tr("No port name specified"));
        return;
    }

    _portUpdated = true;

    while (!_threadQuit)
    {
        //  Check if port name changed while running
        if (_portUpdated)
        {
            serial.close();
            serial.setPortName(_portName);
            serial.setBaudRate(_portBaud.toUInt());

            if (!serial.open(QIODevice::ReadWrite))
            {
                emit logLine(tr("Can't open %1, error code %2")
                           .arg(_portName).arg(serial.error()));
                return;
            }
            _portUpdated = false;
        }

        // Read response
        if (serial.waitForReadyRead(5))
        {
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(1))
                responseData += serial.readAll();

            const QString response = QString::fromUtf8(responseData);

            _mux->ReceiveData(response);
        }
    }
    serial.close();
    _threadQuit = false;
    emit logLine("Serial thread exited");
}
