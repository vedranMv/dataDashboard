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
}

DataMultiplexer::~DataMultiplexer()
{
    //  Pass kill signal to the thread and wait for it to terminate
    _threadQuit = true;
    wait();
}

/**
 * @brief DataMultiplexer::SetSerialFrameFormat
 * @param start
 * @param delim
 * @param end
 */
void DataMultiplexer::SetSerialFrameFormat(const char &start, const char &delim, const char &end)
{
    _SerialframeFormat[0] = start;
    _SerialframeFormat[1] = delim;
    _SerialframeFormat[2] = end;

    logLine(QString("Updated frame format with ")+_SerialframeFormat[0]+_SerialframeFormat[1]+_SerialframeFormat[2]);
}

void DataMultiplexer::RegisterSerialCh(uint8_t n, QString *chName)
{
    _SerialLabels.clear();
    qDebug() << "Attempting to reconfigure";

    for (uint8_t i = 0; i < n; i++)
        _SerialLabels.push_back(chName[i]);
    qDebug() << "Attempting to reconfigure2";


    _channelCount[SignalSource::SerialSignal] = n;
    qDebug() << "Attempting to reconfigure3";

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
    //  Clean up old variable
    _mChannel[channelId-1].Clear();

    //  Move new data in
    _mChannel[channelId-1] = *mc;
    //  Enable math channel
    _mChannel[channelId-1].Enabled = true;
    _channelCount[SignalSource::MathSignal]++;

    _InternalChannelUpdate();
    emit ChannelsUpdated();

}

/**
 * @brief DataMultiplexer::UnegisterMathChannel
 * @param channelId Math channel (1-indexed)
 */
void DataMultiplexer::UnegisterMathChannel(uint8_t channelId)
{
    _mChannel[channelId-1].Clear();
    _channelCount[SignalSource::MathSignal]--;

    _InternalChannelUpdate();
    emit ChannelsUpdated();
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

    for (MathChannel &X : _mChannel)
        if (X.Enabled)
            retVal.append(X.GetLabel());

   qDebug() << retVal;

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

    //  If _channelData has been initialized before, delete it first
    if (_channelCount[SignalSource::AllChannels] != 0)
        delete [] _channelData;

    //  Compute new number total of channels
    _channelCount[SignalSource::AllChannels] =
            _channelCount[SignalSource::SerialSignal] +
            _channelCount[SignalSource::MathSignal];

    //  Allocate data array for all current channels
    _channelData = new double[_channelCount[SignalSource::AllChannels]];

}

/**
 * @brief DataMultiplexer::RegisterGraph
 * @param name
 * @param nInChannels
 * @param receiver
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                OrientationWidget* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
}

/**
 * @brief DataMultiplexer::RegisterGraph
 * @param name
 * @param nInChannels
 * @param receiver
 */
void DataMultiplexer::RegisterGraph(QString name,
                                uint8_t nInChannels,
                                ScatterDataModifier* receiver)
{
    _Graphs.push_back(new GraphClient(name,nInChannels,receiver));
    emit logLine("Registered graph "+name);
}

/**
 * @brief DataMultiplexer::UnregisterGraph
 * @param reciver
 */
void DataMultiplexer::UnregisterGraph(OrientationWidget* reciver)
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

void DataMultiplexer::UnregisterGraph(ScatterDataModifier* reciver)
{
    for (uint8_t i = 0; i < _Graphs.size(); i++)
    {
        if (_Graphs[i]->Receiver(reciver) == reciver)
        {
            _Graphs.erase(_Graphs.begin()+i);
            break;
        }
    }
}

/**
 * @brief DataMultiplexer::ComputeMathChannels
 */
void DataMultiplexer::ComputeMathChannels()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        //  Skip channel if it's not enabled
        if (!_mChannel[i].Enabled)
            continue;

        //  Math channels are offset by the count of serial channels
        _channelData[ _channelCount[SignalSource::SerialSignal] + i] = 0;

        for (uint8_t j = 0; j < _mChannel[i]._component.size(); j++)
            if (std::get<0>(_mChannel[i]._component[j]) == MathOperation::Add_Signal)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] += \
                        _channelData[ std::get<1>(_mChannel[i]._component[j]) ];
        else if (std::get<0>(_mChannel[i]._component[j]) == MathOperation::Subtract_Signal)
                _channelData[ _channelCount[SignalSource::SerialSignal] + i] -= \
                        _channelData[ std::get<1>(_mChannel[i]._component[j]) ];
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
    _buffer = buffer;

    if (!isRunning())
    {
        _threadQuit = false;
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
            //  Find start of the frame
            while (buffer[i].cell() != _SerialframeFormat[0])
                //  If we're about to overflow the buffer, terminate further
                //  processing and jump to the end of this loop to wait for
                //  new data frame
                if ((i+1) >= buffer.length())
                    goto end_goto;
                else
                    i++;

            if ((i+1) >= buffer.length())
                goto end_goto;
            else
                i++;

            //  Loop until the end char has been found
            while (buffer[i].cell() != _SerialframeFormat[2])
            {
                tmp += buffer[i];
                if ((i+1) >= buffer.length())
                    goto end_goto;
                else
                    i++;
            }

            QStringList chnValues = tmp.split( _SerialframeFormat[1] );

            //  Check for discrepancy in channel count
            if (chnValues.size() != _SerialLabels.size())
            {
                emit logLine(tr("Channel number discrepancy"));
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

            ComputeMathChannels();
            //  Update graphs
            for (GraphClient* X : _Graphs)
                X->SendData(_channelCount[SignalSource::AllChannels], _channelData);

            for (uint8_t i = 0; i < _channelCount[0]; i++)
                qDebug()<<_channelCount[0]<<":"<<_channelData[0]<<","<<_channelData[1]<<","<<_channelData[2]<<"," \
                        <<_channelData[3]<<","<<_channelData[4];
        }
end_goto:
        _SerialdataReady.release(1);

    }
}
