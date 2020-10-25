#include "orientationwindow.h"

#include <QVBoxLayout>
#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <mainwindow.h>
#include <QQuaternion>


OrientationWindow::OrientationWindow(QWidget *parent)
{
    this->setParent(parent);
    //  Create container window
    _contWind = new QWidget();
    this->setWidget(_contWind);

    //  Main vertical layout
    QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);

    //  Header with input channel drop-downs
    _header = new graphHeaderWidget(3, false);
    windMainLayout->addLayout(_header->GetLayout());

    //  3D orientation widget
    _widget3d = new OrientationWidget();
    _widget3d->setMinimumSize(QSize(200,200));
    _widget3d->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    windMainLayout->addWidget(_widget3d);

    //  Make sure input channel dropdowns have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     _header,
                     &graphHeaderWidget::UpdateChannelDropdown);
    //  Handle dynamic channel selection by dropdown
    QObject::connect(_header, &graphHeaderWidget::UpdateInputChannels,
                     this, &OrientationWindow::UpdateInputChannels);

    _inputChannels[0] = 0;
    _inputChannels[1] = 0;
    _inputChannels[2] = 0;
    _maxInChannel = 0;
}

OrientationWindow::~OrientationWindow()
{
    qDebug()<<"Deleting orientation window";
    DataMultiplexer::GetI().UnregisterGraph(this);

    MainWindow::clearLayout(_contWind->layout());
    _contWind->deleteLater();
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void OrientationWindow::ReceiveData(double *data, uint8_t n)
{
    qDebug()<<"Received data";
    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    qDebug()<<"Enough data. In "<<n;
    for (uint8_t i = 0; i < n; i++)
        qDebug()<<data[i];

    // Update rotation
    _widget3d->rotation = QQuaternion::fromEulerAngles( (float)data[ _inputChannels[1] ],
                                                (float)data[ _inputChannels[2] ],
                                                (float)data[ _inputChannels[0] ] );
    qDebug()<<"Computed rotation";
    _widget3d->update();
    qDebug()<<"Updated widget";
}

/**
 * @brief [Slot] Function that is called whenever input channel has been
 *      changed in the dropdown fields of the header. It updates the channel
 *      selection stored in this object.
 * @param inChannels    Array of 3 input channel indexes
 */
void OrientationWindow::UpdateInputChannels(uint8_t *inChannels)
{
   _inputChannels[0] = inChannels[0];
   _inputChannels[1] = inChannels[1];
   _inputChannels[2] = inChannels[2];

  _maxInChannel = 0;
  for (uint8_t i = 0; i < 3; i++)
      if (inChannels[i] > _maxInChannel)
          _maxInChannel = inChannels[i];
}
