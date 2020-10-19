/**
  *     Data multiplexer singleton (with helper objects)
  *
  *     Thread object used to receive data from multiple sources, parse it,
  *     perform simple math on it and pass further on to the graphs
  */

#ifndef DATAMULTIPLEXER_H
#define DATAMULTIPLEXER_H

#include <QObject>
#include <QString>
#include <vector>
#include <tuple>

#include <orientation_3d/orientationwidget.h>

enum SignalSource {
    AllChannels,
    SerialSignal,
    MathSignal,
    NetworkSignal
};

enum MathOperation {
    Add_Signal,
    Subtract_Signal
};

class GraphClient
{
    friend class DataMultiplexer;
public:
    GraphClient(QString name, uint8_t inCh, OrientationWidget* reciver) \
        :_name(name), _inChannels(inCh), _reciver(reciver)
    {  }

    void SetInputChannels(uint8_t n, uint8_t *chList)
    {
        _inputChannelMap.clear();

        for (uint8_t i = 0; i < n; i++)
            _inputChannelMap.push_back(chList[i]);
    }

    void SendData(uint8_t n, double* data)
    {
        if (n != _inChannels)
            return;
        _reciver->ReceiveData(data, n);
    }

    const QString& GetName()
    {
        return _name;
    }

    OrientationWidget* Receiver()
    {
        return _reciver;
    }

private:
    QString _name;
    uint8_t _inChannels;
    OrientationWidget* _reciver;
    std::vector<uint8_t>_inputChannelMap;
};

class MathChannel
{
public:
    MathChannel(): Enabled(false) {};
    void SetLabel(QString label)
    {
         _label = label;
    }
    QString GetLabel()
    {
         return _label;
    }

    void AddComponent(MathOperation operation, uint8_t serialChannel)
    {
        _component.push_back(std::tuple<MathOperation,uint8_t>(operation,serialChannel));
    }

    void Clear()
    {
        Enabled = false;
        _label = QString("");
        _component.clear();
    }

    bool Enabled;

//private:
    QString _label;
    std::vector< std::tuple<MathOperation,uint8_t> >_component;
};


class DataMultiplexer : public QThread
{

    Q_OBJECT

public:
    //  Get singleton
    static DataMultiplexer& GetI();
    static DataMultiplexer* GetP();

    void SetSerialFrameFormat(const char &start, const char &delim, const char &end);

    void RegisterSerialCh(uint8_t n, QString *chName);

    void ReceiveSerialData(const QString &s);


    void RegisterGraph(QString name,
                       uint8_t nCh,
                       OrientationWidget* reciver);

    void UnregisterGraph(OrientationWidget* reciver);

signals:
    void logLine(const QString &s);
    void ChannelsUpdated();

public slots:



    void RegisterMathChannel(uint8_t channelId,
                             QString label,
                             uint8_t *operations,
                             uint8_t *serialChannels,
                             uint8_t n);
    void RegisterMathChannel(uint8_t channelId,
                             MathChannel *mc);
    void UnegisterMathChannel(uint8_t channelId);

    QStringList GetChannelList();


private:
        DataMultiplexer();
        ~DataMultiplexer();
        void ComputeMathChannels();
        void _InternalChannelUpdate();

        void run() override;
        bool _threadQuit;

        char _SerialframeFormat[3];

        double *_Serialdata;
        double _MathData[7];

        double *_channelData;
        uint8_t _channelCount[4];

        MathChannel _mChannel[7];
        std::vector<QString>_SerialLabels;
        std::vector<GraphClient>_Graphs;

        QString _buffer;
        QSemaphore _SerialdataReady;
};

#endif // DATAMULTIPLEXER_H
