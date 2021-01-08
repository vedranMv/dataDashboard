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

#include "graphclient.h"
#include "mathchannel.h"

/**
 * @brief The DataMultiplexer class
 *      Designed as singleton object with GetI and GetP methods for accessing
 *      the static instance
 *
 *      Handles all the data coming in and out of the program, including all
 *      the manipulation (math) and logging to file
 */
class DataMultiplexer : public QThread
{

    Q_OBJECT

public:
    //  Get singleton
    static DataMultiplexer& GetI();
    static DataMultiplexer* GetP();

    void SetSerialFrameFormat(const QString &start, const QString &delim, const QString &end);

    void RegisterSerialCh(uint8_t n, QString *chName);

    void ReceiveData(const QString &s);


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

    uint16_t GetSampleRateEst();
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

private slots:
    void _TimerTick();

private:
        DataMultiplexer();
        ~DataMultiplexer();
        void _ComputeMathChannels();
        void _InternalChannelUpdate();

        void run() override;
        bool _threadQuit;

        QString _SerialframeFormat[3];

        double *_channelData;
        uint8_t _channelCount[4];

        MathChannel* _mChannel[7];
        std::vector<QString>_SerialLabels;
        std::vector<GraphClient*>_Graphs;

        QString _buffer;
        QSemaphore _InputdataReady;

        QFile *_logFile;
        QTextStream *_logFileStream;
        bool _logToFile;
        char _logChSep;

        uint16_t _sampleCount;
        uint16_t _extSampleCount;
        QTimer  _timer;
};

#endif // DATAMULTIPLEXER_H
