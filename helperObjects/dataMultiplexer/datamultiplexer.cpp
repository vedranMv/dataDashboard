#include "datamultiplexer.h"
#include <QDebug>

DataMultiplexer::DataMultiplexer(): _SerialChannelCount(0)
{

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

    qDebug() << "Updated frame format with" << _SerialframeFormat[0] << _SerialframeFormat[1] << _SerialframeFormat[2];
}

void DataMultiplexer::SetSerialDataLabels(uint8_t n, QString *chName)
{
    _SerialLabels.clear();

    for (uint8_t i = 0; i < n; i++)
        _SerialLabels.push_back(chName[i]);

    _SerialChannelCount = n;
    _Serialdata = new double[n];
}

/**
 * @brief DataMultiplexer::ComputeMathChannels
 */
void DataMultiplexer::ComputeMathChannels()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        _MathData[i] = 0;

        //  Skip channel if it's not enabled
        if (!_mChannel[i].Enabled)
            continue;

        for (uint8_t j = 0; j < _mChannel[i]._component.size(); j++)
            if (std::get<0>(_mChannel[i]._component[j]) == MathOperation::Add_Signal)
                _MathData[i] += _Serialdata[ std::get<1>(_mChannel[i]._component[j]) ];
        else if (std::get<0>(_mChannel[i]._component[j]) == MathOperation::Subtract_Signal)
                _MathData[i] -= _Serialdata[ std::get<1>(_mChannel[i]._component[j]) ];
    }
}

/**
 * @brief RegisterMathChannel
 * @param channelId Math channel id (1-indexed)
 * @param label
 * @param operations
 * @param serialChannels
 * @param n
 */
void DataMultiplexer::RegisterMathChannel(uint8_t channelId,
                                          QString label,
                                          uint8_t *operation,
                                          uint8_t *serialChannels,
                                          uint8_t n)
{
    _mChannel[channelId-1].Clear();

    for (uint8_t i = 0; i < n; i++)
    {
        _mChannel[channelId-1].SetLabel(label);
        _mChannel[channelId-1].AddComponent(static_cast<MathOperation>(operation[i]), serialChannels[i]);
    }
    _mChannel[channelId-1].Enabled = true;
    qDebug() << "Registered channel " << channelId << "with " << n << "components";
}

/**
 * @brief DataMultiplexer::UnegisterMathChannel
 * @param channelId Math channel (1-indexed)
 */
void DataMultiplexer::UnegisterMathChannel(uint8_t channelId)
{
    _mChannel[channelId-1].Clear();
}

/**
 * @brief [Slot] Parser for data from serial port
 * @param s
 */
void DataMultiplexer::ReceiveSerialData(const QString &buffer)
{

    //  Sanity check, has this been properly initialized?
    if (_SerialChannelCount == 0)
    {
        emit error(tr("Channel number discrepancy"));
        return;
    }

    //  Loop through received data and split it in frames
    for (int32_t i = 0; i < buffer.length(); i++)
    {
        QString tmp("");
        //  Find start of the frame
        while (buffer[i].cell() != _SerialframeFormat[0])
            if ((i+1) >= buffer.length())
                return;
            else
                i++;

        i++;
        //  Loop until the end char has been found
        while (buffer[i].cell() != _SerialframeFormat[2])
        {
            tmp += buffer[i];
            if ((i+1) >= buffer.length())
                return;
            else
                i++;
        }

        QStringList chnValues = tmp.split( _SerialframeFormat[1] );

        //  Check for discrepancy in channel count
        if (chnValues.size() != _SerialChannelCount)
        {
            emit error(tr("Channel number discrepancy"));
            return;
        }

        //  Go through the list of channels and move it into data buffer
        for (uint8_t j = 0; j < chnValues.size(); j++)
        {
            bool ok = false;
            double tmp = chnValues[j].toDouble(&ok);

            if (ok)
                _Serialdata[j] = tmp;
        }

        ComputeMathChannels();
        qDebug()<<_Serialdata[0]<<" " <<_Serialdata[1]<<" " <<_Serialdata[2]<<" " <<_MathData[0]<<" " <<_MathData[1];
    }
}
