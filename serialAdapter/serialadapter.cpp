/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "serialadapter.h"

#include <QSerialPort>
#include <QTime>

SerialAdapter::SerialAdapter(QObject *parent) :
    QThread(parent)
{
}

SerialAdapter::~SerialAdapter()
{
    portMutex.lock();
    threadQuit = true;
    m_cond.wakeOne();
    portMutex.unlock();
    wait();
}

void SerialAdapter::startThread()
{
//! [1]
    const QMutexLocker locker(&portMutex);

    //  Check if the thread is already running, if not start it
    if (!isRunning())
    {
        threadQuit = false;
        start();
    }
    else
        m_cond.wakeOne();
}

void SerialAdapter::stopThread()
{
    threadQuit = true;
}

void SerialAdapter::updatePort(const QString &portName_, const QString &portBaud_)
{
    if ((portName_ == portName) && (portBaud_ == portBaud))
        portUpdated = false;
    else
    {
        portUpdated = true;
        portName = portName_;
        portBaud = portBaud_;
    }
}

/**
 * @brief Thread to read and publish serial data, capable of dynamically
 *  updating port parameters
 */
void SerialAdapter::run()
{
    QSerialPort serial;

    if (portName.isEmpty())
    {
        emit error(tr("No port name specified"));
        return;
    }

    portUpdated = true;

    while (!threadQuit)
    {
        //  Check if port name changed while running
        if (portUpdated)
        {
            serial.close();
            serial.setPortName(portName);
            serial.setBaudRate(portBaud.toUInt());

            if (!serial.open(QIODevice::ReadWrite))
            {
                emit error(tr("Can't open %1, error code %2")
                           .arg(portName).arg(serial.error()));
                return;
            }
            portUpdated = false;
        }

        // Read response
        if (serial.waitForReadyRead(10))
        {
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(10))
                responseData += serial.readAll();

            const QString response = QString::fromUtf8(responseData);

            emit this->response(response);
        }
    }
    serial.close();
    threadQuit = false;
}
