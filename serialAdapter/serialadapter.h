#ifndef SERIALADAPTER_H
#define SERIALADAPTER_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "helperObjects/dataMultiplexer/datamultiplexer.h"


class SerialAdapter : public QThread
{
    Q_OBJECT

public:
    explicit SerialAdapter(QObject *parent = nullptr);
    ~SerialAdapter();

    void StartListening();
    void StopListening();

signals:
    void logLine(const QString &s);

public slots:
    void UpdatePort(const QString &portName, const QString &portBaud);

private:
    void run() override;

    QString _portName;
    QString _portBaud;
    bool _threadQuit = false;
    bool _portUpdated = false;
    DataMultiplexer* _mux;
};

#endif // SERIALADAPTER_H
