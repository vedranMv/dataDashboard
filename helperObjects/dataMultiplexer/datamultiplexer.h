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
#include <QThread>

#include <orientation_3d/orientationwindow.h>
#include <scatter/scatterwindow.h>
#include <line/lineplot.h>

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

enum GraphType {
    Orientation3D,
    Scatter,
    Line
};


class GraphClient
{
    friend class DataMultiplexer;
public:
    GraphClient(QString name, uint8_t nInChannels, OrientationWindow* reciver) \
        :_name(name), _inChannels(nInChannels), _reciver3D(reciver)
    { _type = GraphType::Orientation3D; }
    GraphClient(QString name, uint8_t nInChannels, ScatterWindow* reciver) \
        :_name(name), _inChannels(nInChannels), _receiverScatter(reciver)
    { _type = GraphType::Scatter; }
    GraphClient(QString name, uint8_t nInChannels, LinePlot* reciver) \
        :_name(name), _inChannels(nInChannels), _receiverLine(reciver)
    { _type = GraphType::Line; }

    void SetInputChannels(uint8_t n, uint8_t *chList)
    {
        _inputChannelMap.clear();

        for (uint8_t i = 0; i < n; i++)
            _inputChannelMap.push_back(chList[i]);
    }

    void SendData(uint8_t n, double* data)
    {
        switch (_type)
        {
        case GraphType::Orientation3D:
            _reciver3D->ReceiveData(data, n);
            break;
        case GraphType::Scatter:
            _receiverScatter->ReceiveData(data, n);
            break;
        case GraphType::Line:
            _receiverLine->ReceiveData(data, n);
            break;
        default:
            break;
        }

    }

    const QString& GetName()
    {
        return _name;
    }

    OrientationWindow* Receiver(OrientationWindow* dummy=0)
    {
        assert(dummy==dummy);
        qDebug()<<"Asked for 3d receiver";
        return _reciver3D;
    }

    ScatterWindow* Receiver(ScatterWindow* dummy=0)
    {
        assert(dummy==dummy);
        qDebug()<<"Asked for scatter receiver";
        return _receiverScatter;
    }

    LinePlot* Receiver(LinePlot* dummy=0)
    {
        assert(dummy==dummy);
        qDebug()<<"Asked for line receiver";
        return _receiverLine;
    }


private:
    GraphType _type;
    QString _name;
    uint8_t _inChannels;
    OrientationWindow* _reciver3D;
    ScatterWindow* _receiverScatter;
    LinePlot    *_receiverLine;
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
                       uint8_t nInChannels,
                       OrientationWindow* reciver);
    void RegisterGraph(QString name,
                       uint8_t nInChannels,
                       ScatterWindow *receiver);
    void RegisterGraph(QString name,
                       uint8_t nInChannels,
                       LinePlot *receiver);

    void UnregisterGraph(OrientationWindow* reciver);
    void UnregisterGraph(ScatterWindow* reciver);
    void UnregisterGraph(LinePlot* reciver);

signals:
    void logLine(const QString &s);
    void ChannelsUpdated();

public slots:

    void RegisterMathChannel(uint8_t channelId,
                             MathChannel *mc);
    void UnegisterMathChannel(uint8_t channelId);

    QStringList GetChannelList();

    int EnableFileLogging(const QString &logPath, bool append, char chSep);
    void DisableFileLogging();


private:
        DataMultiplexer();
        ~DataMultiplexer();
        void _ComputeMathChannels();
        void _InternalChannelUpdate();

        void run() override;
        bool _threadQuit;

        char _SerialframeFormat[3];

        double *_channelData;
        uint8_t _channelCount[4];

        MathChannel* _mChannel[7];
        std::vector<QString>_SerialLabels;
        std::vector<GraphClient*>_Graphs;

        QString _buffer;
        QSemaphore _SerialdataReady;

        QFile *_logFile;
        QTextStream *_logFileStream;
        bool _logToFile;
        char _logChSep;
};

#endif // DATAMULTIPLEXER_H
