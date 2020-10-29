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

public slots:
    void refresh();

private:
    QWidget *_contWind;
    QCustomPlot *_plot;

    uint8_t _inputChannels[3];
    uint8_t _maxInChannel;

    graphHeaderWidget *_header;
    QVector<double> _inputCh[2];
    QVector<double> _xAxis;
    QTimer *_refresher;
};

#endif // LINEPLOT_H
