#ifndef LINEPLOT_H
#define LINEPLOT_H
#include <QVBoxLayout>
#include <QMdiSubWindow>

#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>

class LinePlot : public QMdiSubWindow
{
public:
    LinePlot();
private:
    QWidget *_contWind;
    graphHeaderWidget *header;
};

#endif // LINEPLOT_H
