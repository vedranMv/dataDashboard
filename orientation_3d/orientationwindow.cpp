#include "orientationwindow.h"

#include <QVBoxLayout>
#include <helperObjects/dataMultiplexer/datamultiplexer.h>


OrientationWindow::OrientationWindow(QWidget *parent)
{
    this->setParent(parent);
    //  Create container window
    _contWind = new QWidget();
    this->setWidget(_contWind);

    //  Main vertical layout
    QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);


    //  Header with input channel drop-downs
    graphHeaderWidget *header = new graphHeaderWidget(3, false);
    windMainLayout->addLayout(header->GetLayout());

    //  3D orientation widget
    OrientationWidget *tmp = new OrientationWidget(_contWind);
    tmp->setMinimumSize(QSize(200,200));
    tmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    windMainLayout->addWidget(tmp);

    //  Make sure input channel dropdowns have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     header,
                     &graphHeaderWidget::UpdateChannelDropdown);
    //  Handle dynamic channel selection by dropdown
    QObject::connect(header, &graphHeaderWidget::UpdateInputChannels,
                     tmp, &OrientationWidget::UpdateInputChannels);
}
