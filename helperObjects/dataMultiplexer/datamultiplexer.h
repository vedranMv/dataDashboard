#ifndef DATAMULTIPLEXER_H
#define DATAMULTIPLEXER_H

#include <QObject>
#include <QString>
#include <vector>
#include <tuple>

enum SignalSource {
    InputSignal,
    MathSignal,
    NetworkSignal
};

enum MathOperation {
    Add_Signal,
    Subtract_Signal
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


class DataMultiplexer : public QObject
{

    Q_OBJECT

public:
    DataMultiplexer();

    void SetSerialFrameFormat(const char &start, const char &delim, const char &end);
    void SetSerialDataLabels(uint8_t n, QString *chName);

signals:
    void error(const QString &s);

public slots:
        void ReceiveSerialData(const QString &s);

        void RegisterMathChannel(uint8_t channelId,
                                 QString label,
                                 uint8_t *operations,
                                 uint8_t *serialChannels,
                                 uint8_t n);
        void UnegisterMathChannel(uint8_t channelId);

private:
        void ComputeMathChannels();

        char _SerialframeFormat[3];
        double *_Serialdata;
        double _MathData[7];
        uint8_t _SerialChannelCount;
        MathChannel _mChannel[7];
        std::vector<QString>_SerialLabels;
};

#endif // DATAMULTIPLEXER_H
