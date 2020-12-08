#ifndef LINEPLOT_H
#define LINEPLOT_H
#include <QVBoxLayout>
#include <QMdiSubWindow>

#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>
#include "line/qcustomplot.h"

class LinePlot : public QMdiSubWindow
{
    Q_OBJECT
public:
    LinePlot();
    ~LinePlot();

    void UpdateInputChannels(uint8_t *inChannels);
    void ReceiveData(double *data, uint8_t n);

signals:
    void logLine(const QString &line);

public slots:
    void ChannelAdded();
    void UpdateXaxis(const QString &_datasize);

private slots:
    void _toggleAccumulatedMode(bool state);

private:
    void _ConstructUI();


    uint8_t _nInputs;
    QSemaphore _plotDataMutex;

    QWidget *_contWind;
    QCustomPlot *_plot;
    QVBoxLayout *windMainLayout;

    QVector<uint8_t>_inputChannels;
    uint8_t _maxInChannel;

    graphHeaderWidget *_header;
    QCheckBox *_autoAdjustYaxis;
    QCheckBox *_accumulate;

    QVector < QVector<double> >_inputCh;
    QVector<double> _xAxis;
    QTimer *_refresher;
};

#endif // LINEPLOT_H
