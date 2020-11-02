#include "orientationwindow.h"

#include <QVBoxLayout>
#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <mainwindow.h>
#include <QQuaternion>


OrientationWindow::OrientationWindow(QWidget *parent): _nInputs(3)
{
    this->setParent(parent);
    //  Create container window
    _contWind = new QWidget();
    this->setWidget(_contWind);
    windMainLayout = new QVBoxLayout(_contWind);

    _ConstructUI();
}

OrientationWindow::~OrientationWindow()
{
    qDebug()<<"Deleting orientation window";
    DataMultiplexer::GetI().UnregisterGraph(this);
}

void OrientationWindow::_ConstructUI()
{
    if (!_contWind->layout()->isEmpty())
    {
        DataMultiplexer::GetI().UnregisterGraph(this);
        MainWindow::clearLayout(_contWind->layout());
    }

    //  Header with input channel drop-downs
    _header = new graphHeaderWidget(_nInputs, false);
    windMainLayout->addLayout(_header->GetLayout());

    QVBoxLayout *radioButtonHolder = new QVBoxLayout();

    QRadioButton *rpyInput = new QRadioButton();
    QRadioButton *quatInput = new QRadioButton();
    rpyInput->setText("Euler (RPY)");
    quatInput->setText("Quaternion (w,x,y,z)");
    if (_nInputs == 3)
        rpyInput->setChecked(true);
    else
        quatInput->setChecked(true);
    radioButtonHolder->addWidget(new QLabel("Input type"));
    radioButtonHolder->addWidget(rpyInput);
    radioButtonHolder->addWidget(quatInput);
    radioButtonHolder->addSpacerItem(new QSpacerItem (20,20,QSizePolicy::Expanding));
    _header->GetLayout()->addLayout(radioButtonHolder);
    _header->AppendHorSpacer();


    connect(rpyInput, &QRadioButton::toggled,
            this, &OrientationWindow::InputTypeUpdated);

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

    for (uint8_t i = 0; i < _nInputs; i++)
        _inputChannels[i] = 0;

    _maxInChannel = 0;

    //  Register with the mux
    DataMultiplexer::GetI().RegisterGraph(this->objectName(), _nInputs, this);
}

void OrientationWindow::InputTypeUpdated(bool rpySelected)
{
    if (rpySelected)
        _nInputs = 3;
    else
        _nInputs = 4;

    _ConstructUI();
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void OrientationWindow::ReceiveData(double *data, uint8_t n)
{

    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    // Update rotation
    if (_nInputs == 3)
        _widget3d->rotation = \
                QQuaternion::fromEulerAngles( (float)data[ _inputChannels[1] ],
                                                    (float)data[ _inputChannels[2] ],
                                                    (float)data[ _inputChannels[0] ] );
    else
        _widget3d->rotation = \
                QQuaternion((float)data[ _inputChannels[0] ],
                (float)data[ _inputChannels[1] ],
                (float)data[ _inputChannels[2] ],
                (float)data[ _inputChannels[3] ] );

    _widget3d->update();;
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
