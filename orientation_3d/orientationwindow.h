#ifndef ORIENTATIONWINDOW_H
#define ORIENTATIONWINDOW_H

#include <QMdiSubWindow>
#include <QWidget>
#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>
#include "orientation_3d/orientationwidget.h"


class OrientationWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    OrientationWindow(QWidget *parent, QString objName="");
    ~OrientationWindow();

    void ReceiveData(double *data, uint8_t n);

signals:
    void logLine(const QString &line);

public slots:
    void UpdateInputChannels(uint8_t *inChannels);
    void InputTypeUpdated(bool rpySelected);

private:
    void _ConstructUI();
    QWidget             *_contWind;
    QVBoxLayout         *windMainLayout;
    graphHeaderWidget   *_header;
    OrientationWidget   *_widget3d;
    uint8_t             _inputChannels[4];
    uint8_t             _maxInChannel;
    uint8_t             _nInputs;
};

#endif // ORIENTATIONWINDOW_H
