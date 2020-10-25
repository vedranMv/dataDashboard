#ifndef ORIENTATIONWINDOW_H
#define ORIENTATIONWINDOW_H

#include <QMdiSubWindow>
#include <QWidget>
#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>
#include "orientation_3d/orientationwidget.h"


class OrientationWindow : public QMdiSubWindow
{
public:
    OrientationWindow(QWidget *parent);
    ~OrientationWindow();

    void ReceiveData(double *data, uint8_t n);

public slots:
    void UpdateInputChannels(uint8_t *inChannels);

private:
    QWidget             *_contWind;
    graphHeaderWidget   *_header;
    OrientationWidget   *_widget3d;
    uint8_t             _inputChannels[3];
    uint8_t             _maxInChannel;
};

#endif // ORIENTATIONWINDOW_H
