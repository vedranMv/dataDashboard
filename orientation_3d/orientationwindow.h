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

    void UpdateInputChannels(uint8_t *inChannels);
    void ReceiveData(double *data, uint8_t n);


private:
    QWidget *_contWind;

};

#endif // ORIENTATIONWINDOW_H
