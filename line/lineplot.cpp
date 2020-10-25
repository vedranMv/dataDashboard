#include "lineplot.h"

LinePlot::LinePlot()
{
     _contWind = new QWidget();

     //  Main vertical layout
     QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);
     //this->setLayout(windMainLayout);
     this->setWidget(_contWind);

     header = new graphHeaderWidget(3, false);
     windMainLayout->addLayout(header->GetLayout());


}
