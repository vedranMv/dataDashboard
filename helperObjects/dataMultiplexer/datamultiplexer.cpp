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
 * @brief Register orientation plot
 * @param name Name of the plot
 * @param nInChannels Number of input channels
 * @param receiver Pointer to graph object
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                OrientationWindow* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}
/**
 * @brief Register scatter plot
 * @param name Name of the plot
 * @param nInChannels Number of input channels
 * @param receiver Pointer to graph object
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                ScatterWindow* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}
/**
 * @brief Register line plot
 * @param name Name of the plot
 * @param nInChannels Number of input channels
 * @param receiver Pointer to graph object
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                LinePlot* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}


/**
 * @brief Unregister orientation plot
 * @param reciver Pointer to graph object
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
/**
 * @brief Unregister scatter plot
 * @param reciver Pointer to graph object
 */
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
/**
 * @brief Unregister line plot
 * @param reciver Pointer to graph object
 */
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

/**
 * @brief Enable logging data into a file
 * @param logPath System path to log file
 * @param append If true, append data to existing file. If false, overwrite
 *      existing file
 * @param chSep Channel separator character
 * @return 0 on success, -1 otherwise
 */
int DataMultiplexer::EnableFileLogging(const QString &logPath, bool append, char chSep)
{
    emit logLine("Enabling logging to file "+logPath);
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

/**
 * @brief Disabling logging to file
 */
void DataMultiplexer::DisableFileLogging()
{
    logLine("Waiting on mutex to disable file logging");

    _logToFile = false;
    _SerialdataReady.acquire(2);


    _logFile->close();
    delete _logFileStream;
    _logFile->deleteLater();

    _SerialdataReady.release(1);

    logLine("File logging disabled");
}


/**
 * @brief Compute math channel values from input values
 *      Called from data handling thread
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
        else if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Multiply)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] *= \
                        _channelData[ std::get<1>(_mChannel[i]->_component[j]) ];
        else if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Add_Abs)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] += \
                        abs(_channelData[ std::get<1>(_mChannel[i]->_component[j]) ]);
        else if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Subtract_Abs)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] -= \
                        abs(_channelData[ std::get<1>(_mChannel[i]->_component[j]) ]);
        else if (std::get<0>(_mChannel[i]->_component[j]) == MathOperation::Multiply_Abs)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] *= \
                        abs(_channelData[ std::get<1>(_mChannel[i]->_component[j]) ]);
    }
}

/**
 * @brief [Slot] Entry point for incoming serial data
 * @param buffer buffer of received data that's passed to the main thread
 * for parsing and dispatching
 */
void DataMultiplexer::ReceiveSerialData(const QString &buffer)
{

    _SerialdataReady.acquire(1);

    //  Buffer should never go over 4k, otherwise somethign is wrong
    if (_buffer.length() + buffer.length() < 4000)
        _buffer += buffer;
    else
        _buffer = buffer;

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
            _SerialdataReady.release(1);
            return;
        }

        QString tmp("");
        //  Loop through received data and split it in frames. 'i' holds
        //  current char position and is further incremented inside the loop
        for (int32_t i = 0; i < buffer.length(); i++)
        {
            //  tmp holds all characters of the current frame we've processes
            //  If that frame is incomplete, we can push it into the processing
            //   on the next iteration of the loop with new data.
            tmp = "";

            //  If provided, find starting character of the frame
            if (_SerialframeFormat[0].length() == 1)
            {
                //  Loop until we reach the last char in starting sequence
                while (buffer[i++].cell() == _SerialframeFormat[0][0].cell())
                {
                    tmp += buffer[i-1];

                    //  If we're about to look outside the buffer, terminate further
                    //  processing and jump outside of this loop to wait for
                    //  new data frame
                    if ((i+1) >= buffer.length())
                    {
                        //  Save current part fo the frame under assumption that it
                        //  might get completed with new data coming in
                        _buffer = tmp;
                        goto end_goto;
                    }
                }
            }

            //  Used to save strings containing numeric values of each channel
            //  in the frame
            QStringList chnValues;
            //  Look for terminating sequance of a frame in buffer
            if (_SerialframeFormat[2].length() > 0)
            {
                QString tmpNumber = "";
                //  ffIter keeps track of the current character in frame
                //  counter. It's incremented every time current char in frame
                //  counter is found in the buffer, and immediately reset to 0
                //  when there's no match
                uint8_t ffIter = 0;

                //  Loop until the _SerialframeFormat[2] is found in buffer
                //  This block essentially tries to find a substring
                //  _SerialframeFormat[2] in buffer.
                while (ffIter < _SerialframeFormat[2].length())
                {
                    //  Save data in temporary buffer in case this frame
                    //  is not complete
                    tmp += buffer[i];

                    // In case where frame format characters are subset of
                    //  values in the data, this will fail. Maybe future
                    //  consideration?
                    if (buffer[i].cell() == _SerialframeFormat[2][ffIter].cell())
                        ffIter++;
                    else
                    {
                        ffIter=0;
                        //  Check if current char is channel separator, if so
                        //  add the value we just assembled to the string list
                        if (buffer[i] == _SerialframeFormat[1])
                        {
                            chnValues.append(tmpNumber);
                            tmpNumber = "";
                        }
                        else
                        //  Otherwise, just keep adding chars from the buffer
                        //  into the number accumulator
                        {
                            tmpNumber += buffer[i];
                        }
                    }

                    //  If we're about to look outside the buffer, terminate further
                    //  processing and jump outside of this loop to wait for
                    //  new data frame
                    if ((i+1) >= buffer.length() &&
                        (ffIter != _SerialframeFormat[2].length()))
                    {
                        //  To get in here, we must've found the starting sequence
                        //  of the frame but not the ending one. In this case, we
                        //  want to save this partial frame so that the missing
                        //  piece is appended next time new serial data is available

                        //  Save buffer for next loop iteration
                        _buffer = tmp;
                        //emit logLine("Failed to match terminator sequence");
                        goto end_goto;
                    }
                    else
                    {
                        i++;
                    }
                }
                //  We can only get to here if the terminating sequence has
                //  been found. Add the last number detected before the sequence
                chnValues.append(tmpNumber);
            }
            //  Check for discrepancy in channel count
            if (chnValues.size() != (int)_SerialLabels.size())
            {
                emit logLine(tr("Channel number discrepancy. Got %1, expected %2")\
                             .arg(chnValues.size()).arg(_SerialLabels.size()));
                continue;
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
                for (uint8_t k = 0; k < _channelCount[SignalSource::AllChannels]; k++)
                {
                    line += QString::number(_channelData[k]);
                    if ((k+1) < _channelCount[SignalSource::AllChannels])
                        line += _logChSep;
                }
                (*_logFileStream) << line << Qt::endl;
            }

        }
        //  We've reached the end of buffer and successfully processed
        //  everything inside, clear it
        _buffer = "";
end_goto:

        _SerialdataReady.release(1);

    }
    emit logLine("Data thread stopped");
}
