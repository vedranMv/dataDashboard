#include "datamultiplexer.h"
#include <QDebug>

DataMultiplexer& DataMultiplexer::GetI()
{
    static DataMultiplexer inst;
    return inst;
}

DataMultiplexer* DataMultiplexer::GetP()
{
    return &(GetI());
}

DataMultiplexer::DataMultiplexer(): _threadQuit(false)
{
    _SerialdataReady.release();

    for (uint8_t i=0; i < 7; i++)
        _mChannel[i] = new MathChannel();

    _logFile = nullptr;
    _logFileStream = nullptr;
    _logToFile = false;
}

DataMultiplexer::~DataMultiplexer()
{
    //  Pass kill signal to the thread and wait for it to terminate
    _threadQuit = true;
    wait();

    for (uint8_t i=0; i < 7; i++)
        delete _mChannel[i];

    delete [] _channelData;
}

/**
 * @brief DataMultiplexer::SetSerialFrameFormat
 * @param start
 * @param delim
 * @param end
 */
void DataMultiplexer::SetSerialFrameFormat(const QString &start, const QString &delim, const QString &end)
{
    _SerialframeFormat[0] = start;
    _SerialframeFormat[1] = delim;
    _SerialframeFormat[2] = end;

    logLine(QString("Updated frame format with ")+
            _SerialframeFormat[0].length()+
            _SerialframeFormat[1].length()+
            _SerialframeFormat[2].length());
}

void DataMultiplexer::RegisterSerialCh(uint8_t n, QString *chName)
{
    emit logLine(tr("Registering %1 channels").arg(n));
    _SerialLabels.clear();

    for (uint8_t i = 0; i < n; i++)
        _SerialLabels.push_back(chName[i]);
     emit logLine("Adding new labels");


    _channelCount[SignalSource::SerialSignal] = n;
    emit logLine("Updating serial channel count");

    _InternalChannelUpdate();

    emit ChannelsUpdated();

}

/**
 * @brief RegisterMathChannel
 * @param channelId Math channel id (1-indexed)
 * @param mc
 */
void DataMultiplexer::RegisterMathChannel(uint8_t channelId,
                                          MathChannel *mc)
{
    emit logLine(tr("Registering channel %1").arg(channelId));
    _threadQuit = true;
    wait();

    //  Clean up old variable
    delete _mChannel[channelId-1];

    //  Move new data in
    _mChannel[channelId-1] = mc;
    //  Enable math channel
    _mChannel[channelId-1]->Enabled = true;
    _channelCount[SignalSource::MathSignal]++;

    _InternalChannelUpdate();
    emit ChannelsUpdated();

    emit logLine(tr("Finished registering channel %1").arg(channelId));
    _threadQuit = false;
}

/**
 * @brief DataMultiplexer::UnegisterMathChannel
 * @param channelId Math channel (1-indexed)
 */
void DataMultiplexer::UnegisterMathChannel(uint8_t channelId)
{
    emit logLine(tr("Unregistering channel %1").arg(channelId));

    _threadQuit = true;
    wait();

    _mChannel[channelId-1]->Clear();
    _channelCount[SignalSource::MathSignal]--;

    _InternalChannelUpdate();
    emit ChannelsUpdated();

    emit logLine(tr("Finished unregistering channel %1").arg(channelId));
    _threadQuit = false;
}

/**
 * @brief Assemble a list of available input channels. Usually called by the
 *      graphs to know which channels to the user
 * @return QStringList with all channel labels
 */
QStringList DataMultiplexer::GetChannelList()
{
    QStringList retVal;

    for (QString &X : _SerialLabels)
        retVal.append(X);

    for (MathChannel* X : _mChannel)
        if (X->Enabled)
            retVal.append(X->GetLabel());

    return retVal;
}

/**
 * @brief Perform internal update of available input channels
 */
void DataMultiplexer::_InternalChannelUpdate()
{
    //  This can't be done with a running thread. Stop it beforehand
    if (isRunning())
    {
        _threadQuit = true;
        wait();
    }

    emit logLine(tr("Deleting channel list of size %1").arg(_channelCount[SignalSource::AllChannels]));

    //  TODO: This breaks heap boundary when ran with active serial port?
    //  If _channelData has been initialized before, delete it first
//    if (_channelCount[SignalSource::AllChannels] != 0)
//        delete [] _channelData;


    emit logLine(tr("Deleting channel list of size %2"));
    //  Compute new number total of channels
    _channelCount[SignalSource::AllChannels] =
            _channelCount[SignalSource::SerialSignal] +
            _channelCount[SignalSource::MathSignal];

    emit logLine(tr("Allocating %1 new channels").arg(_channelCount[SignalSource::AllChannels]));
    //  Allocate data array for all current channels
    _channelData = new double[_channelCount[SignalSource::AllChannels]];

    //  Update finished, remember to revive the thread
    _threadQuit = false;
}

/**
 * @brief DataMultiplexer::RegisterGraph
 * @param name
 * @param nInChannels
 * @param receiver
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                OrientationWindow* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}

/**
 * @brief DataMultiplexer::RegisterGraph
 * @param name
 * @param nInChannels
 * @param receiver
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                ScatterWindow* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}

void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                LinePlot* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}


/**
 * @brief DataMultiplexer::UnregisterGraph
 * @param reciver
 */
void DataMultiplexer::UnregisterGraph(OrientationWindow* reciver)
{
    QString name("");
    for (uint8_t i = 0; i < _Graphs.size(); i++)
    {
        if (_Graphs[i]->Receiver(reciver) == reciver)
        {
            name = _Graphs[i]->_name;
            _Graphs.erase(_Graphs.begin()+i);
            break;
        }
    }
    emit logLine("Unregistered graph "+name);
}

void DataMultiplexer::UnregisterGraph(ScatterWindow* reciver)
{
    QString name("");
    for (uint8_t i = 0; i < _Graphs.size(); i++)
    {
        if (_Graphs[i]->Receiver(reciver) == reciver)
        {
            name = _Graphs[i]->_name;
            _Graphs.erase(_Graphs.begin()+i);
            break;
        }
    }
    emit logLine("Unregistered graph "+name);
}

void DataMultiplexer::UnregisterGraph(LinePlot* reciver)
{
    QString name("");
    for (uint8_t i = 0; i < _Graphs.size(); i++)
    {
        if (_Graphs[i]->Receiver(reciver) == reciver)
        {
            name = _Graphs[i]->_name;
            _Graphs.erase(_Graphs.begin()+i);
            break;
        }
    }
    emit logLine("Unregistered graph "+name);
}

int DataMultiplexer::EnableFileLogging(const QString &logPath, bool append, char chSep)
{
    _logFile = new QFile(logPath);

    QIODevice::OpenMode flags = QIODevice::WriteOnly | QIODevice::Text;
    if (append)
        flags |= QIODevice::Append;

    if (!_logFile->open(flags))
    {
        emit logLine("Error opening log file");
        _logFile->deleteLater();
        return -1;
    }

    _logFileStream = new QTextStream(_logFile);
    _logChSep = chSep;

    _logToFile = true;
    return 0;
}
void DataMultiplexer::DisableFileLogging()
{
    _logToFile = false;
    _SerialdataReady.acquire(2);


    _logFile->close();
    delete _logFileStream;
    _logFile->deleteLater();

    _SerialdataReady.release(1);
}


/**
 * @brief DataMultiplexer::ComputeMathChannels
 */
void DataMultiplexer::_ComputeMathChannels()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        //  Skip channel if it's not enabled
        if (!_mChannel[i]->Enabled)
            continue;

        //  Math channels are offset by the count of serial channels
        _channelData[ _channelCount[SignalSource::SerialSignal] + i] = 0;

        for (uint8_t j = 0; j < _mChannel[i]->_component.size(); j++)
            if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Add_Signal)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] += \
                        _channelData[ std::get<1>(_mChannel[i]->_component[j]) ];
        else if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Subtract_Signal)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] -= \
                        _channelData[ std::get<1>(_mChannel[i]->_component[j]) ];
    }
}

/**
 * @brief [Slot] Entry point for incoming serial data
 * @param buffer buffer of received data that's passed to the main thread
 * for parsing and dispatching
 */
void DataMultiplexer::ReceiveSerialData(const QString &buffer)
{
    //emit logLine("Waiting to acquire");
    _SerialdataReady.acquire(1);
    _buffer += buffer;
    //emit logLine("Acquired, running");
    if (!isRunning() && !_threadQuit)
    {
        //  External process use this flag to control thread, avoid
        //  forcing its value here
        //_threadQuit = false;
        start();
    }

    _SerialdataReady.release(2);
}

/**
 * @brief Multiplexer main thread
 * This thread is started upon constructing the multiplexer, and is spun up
 * to listen for incoming data, compute math channels and update data in the
 * registered graphs
 */
void DataMultiplexer::run()
{
    emit logLine("Data thread started");
    while (!_threadQuit)
    {
        if (!_SerialdataReady.tryAcquire(2,1))
            continue;

        QString buffer = _buffer;
        //  Sanity check, has this been properly initialized?
        if (_SerialLabels.size() == 0)
        {
            emit logLine(tr("No serial channels registered"));
            goto end_goto;
        }

        //  Loop through received data and split it in frames
        for (int32_t i = 0; i < buffer.length(); i++)
        {
            QString tmp("");
            //  Find start of the frame, if provided
            if (_SerialframeFormat[0].length() != 0)
            {
                uint8_t ffCounter = 0;
                while (ffCounter < _SerialframeFormat[0].length())
                {

                    if (buffer[i].cell() == _SerialframeFormat[0][ffCounter].cell())
                        ffCounter++;
                    else
                        ffCounter=0;
                    //  If we're about to look outside the buffer, terminate further
                    //  processing and jump to the end of this loop to wait for
                    //  new data frame
                    if ((i+1) >= buffer.length())
                        goto end_goto;
                    else
                        i++;
                }
            }

            QStringList chnValues;

            if (_SerialframeFormat[2].length() != 0)
            {
                QString tmpNumber = "";
                uint8_t ffCounter = 0;
                while (ffCounter < _SerialframeFormat[2].length())
                {
                    //  Save data in this temporary buffer in case this frame
                    //  is not complete
                    tmp += buffer[i];
                    // In case where frame format characters are subset of values in the data,
                    //  this will fail. Maybe future consideration?
                    if (buffer[i].cell() == _SerialframeFormat[2][ffCounter].cell())
                        ffCounter++;
                    else
                    {
                        //  Add only data chars here, not frame format characters
                        ffCounter=0;
                        if (buffer[i] == _SerialframeFormat[1])
                        {
                            chnValues.append(tmpNumber);
                            tmpNumber = "";
                        }
                        else
                            tmpNumber += buffer[i];
                    }
                    //  If we're about to look outside the buffer, terminate further
                    //  processing and jump to the end of this loop to wait for
                    //  new data frame
                    if ((i+1) >= buffer.length() && (ffCounter != _SerialframeFormat[2].length()))
                    {
                        //  save buffer for next loop iteration
                        _buffer = tmp;
                        emit logLine("Failed to match terminator sequence");
                        qDebug()<<"i-2: "<<buffer[i-2]<<"i-1: "<<buffer[i-1]<<"i: "<<buffer[i];
                        goto end_goto;
                    }
                    else
                        i++;
                }
                //  We can only get to here if the terminating sequence has
                //  been found. Add last number
                chnValues.append(tmpNumber);
                if (chnValues.size() == 4)
                    qDebug()<<"A";
            }

            //QStringList chnValues = tmp.split( _SerialframeFormat[1] );

            //  Check for discrepancy in channel count
            if (chnValues.size() != _SerialLabels.size())
            {
                emit logLine(tr("Channel number discrepancy. Got %1, expected %2").arg(chnValues.size()).arg(_SerialLabels.size()));
                goto end_goto;
            }

            //  Go through the list of channels and move it into data buffer
            for (uint8_t j = 0; j < chnValues.size(); j++)
            {
                bool ok = false;
                double tmp = chnValues[j].toDouble(&ok);

                if (ok)
                    _channelData[j] = tmp;
                else
                {
                    _channelData[j] = 0.0;
                    emit logLine(tr("Error converting to double"));
                    continue;
                }
            }

            _ComputeMathChannels();
            //  Update graphs
            for (GraphClient* X : _Graphs)
                X->SendData(_channelCount[SignalSource::AllChannels], _channelData);
            //  Log to file if enabled
            if (_logToFile)
            {
                QString line = "";
                for (uint8_t i = 0; i < _channelCount[SignalSource::AllChannels]; i++)
                {
                    line += QString::number(_channelData[i]);
                    if ((i+1) < _channelCount[SignalSource::AllChannels])
                        line += _logChSep;
                }
                (*_logFileStream) << line << Qt::endl;
                qDebug()<<line;
            }
//            for (uint8_t i = 0; i < _channelCount[0]; i++)
//                qDebug()<<_channelCount[0]<<":"<<_channelData[0]<<","<<_channelData[1]<<","<<_channelData[2]<<"," \
//                        <<_channelData[3]<<","<<_channelData[4];
        }
end_goto:
        _SerialdataReady.release(1);

    }
    emit logLine("Data thread stopped");
}
