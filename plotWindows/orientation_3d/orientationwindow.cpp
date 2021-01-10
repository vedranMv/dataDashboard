#include "orientationwindow.h"

#include <QVBoxLayout>
#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <mainwindow.h>
#include <QQuaternion>


OrientationWindow::OrientationWindow(QWidget *parent, QString objName): _nInputs(3)
{
    this->setParent(parent);
    this->setObjectName(objName);
    //  Create container window
    _contWind = new QWidget();
    this->setWidget(_contWind);
    windMainLayout = new QVBoxLayout(_contWind);

    _ConstructUI();
}

OrientationWindow::~OrientationWindow()
{
    emit logLine("3D: Destroying the plot");
    DataMultiplexer::GetI().UnregisterGraph(this);

    disconnect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     _header,
                     &graphHeaderWidget::UpdateChannelDropdown);
    MainWindow::clearLayout(_contWind->layout());
}

/**
 * @brief Constructs UI based on configuration variables
 *      Separated from the constructor to allow layout changes on runtime
 */
void OrientationWindow::_ConstructUI()
{
    //  Check if UI is already been constructed, then destroy it
    if (!_contWind->layout()->isEmpty())
    {
        emit logLine("3D: Deconstructing existing UI");
        DataMultiplexer::GetI().UnregisterGraph(this);
        //  Make sure input channel drop-downs have updated list of channels
        disconnect(DataMultiplexer::GetP(),
                         &DataMultiplexer::ChannelsUpdated,
                         _header,
                         &graphHeaderWidget::UpdateChannelDropdown);
        MainWindow::clearLayout(_contWind->layout());
    }

    QString mode = "";
    if (_nInputs == 3)
        mode = "Euler mode";
    else
        mode = "Quaternion mode";

    emit logLine("3D: Constructing new UI in " + mode);

    //  Basic header with input channel drop-downs
    _header = new graphHeaderWidget(_nInputs, this->objectName());
    windMainLayout->addLayout(_header->GetLayout());

    //  Line to separate channels from config
    QFrame *_vertLine = new QFrame();
    _vertLine->setFrameShape(QFrame::VLine);
    _vertLine->setFrameShadow(QFrame::Sunken);
    _header->GetLayout()->addWidget(_vertLine);

    //  Header items for orientation plot
    QVBoxLayout *orientationSpecificHeader  = new QVBoxLayout();
    _header->GetLayout()->addLayout(orientationSpecificHeader);
    _header->AppendHorSpacer();

    //  Radio buttons for switching between euler and quat input
    QRadioButton *rpyInput = new QRadioButton();
    rpyInput->setText("Euler (RPY)");
    rpyInput->setToolTip(_STYLE_TOOLTIP_(\
                    "Orientation is supplied as Euler angles in degrees"));

    QRadioButton *quatInput = new QRadioButton();
    quatInput->setText("Quaternion (w,x,y,z)");
    quatInput->setToolTip(_STYLE_TOOLTIP_(\
                    "Orientation is supplied as a normalized quaternion"));

    //  Add radio buttons to the header widget
    if (_nInputs == 3)
        rpyInput->setChecked(true);
    else
        quatInput->setChecked(true);
    orientationSpecificHeader->addWidget(new QLabel("Input type"));
    orientationSpecificHeader->addWidget(rpyInput);
    orientationSpecificHeader->addWidget(quatInput);
    orientationSpecificHeader->addSpacerItem(new QSpacerItem (20,20,QSizePolicy::Expanding));

    connect(rpyInput, &QRadioButton::toggled,
            this, &OrientationWindow::InputTypeUpdated);

    //  3D orientation widget
    _widget3d = new OrientationWidget();
    _widget3d->setMinimumSize(QSize(200,200));
    _widget3d->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    windMainLayout->addWidget(_widget3d);

    //  Make sure input channel drop-downs have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     _header,
                     &graphHeaderWidget::UpdateChannelDropdown);
    //  Handle dynamic channel selection by drop-down
    QObject::connect(_header, &graphHeaderWidget::UpdateInputChannels,
                     this, &OrientationWindow::UpdateInputChannels);

    for (uint8_t i = 0; i < _nInputs; i++)
        _inputChannels[i] = 0;

    _maxInChannel = 0;

    //  Update channel names in the header
    if (_nInputs == 3)
    {
        _header->GetLabels()[0]->setText("Roll");
        _header->SetChToolTip(0, _STYLE_TOOLTIP_("Cube roll"));
        _header->GetLabels()[1]->setText("Pitch");
        _header->SetChToolTip(1, _STYLE_TOOLTIP_("Cube pitch"));
        _header->GetLabels()[2]->setText("Yaw");
        _header->SetChToolTip(2, _STYLE_TOOLTIP_("Cube yaw"));
    }
    else if (_nInputs == 4)
    {
        _header->GetLabels()[0]->setText("w");
        _header->SetChToolTip(0, _STYLE_TOOLTIP_("Quaternion w component"));
        _header->GetLabels()[1]->setText("x");
        _header->SetChToolTip(1, _STYLE_TOOLTIP_("Quaternion x component"));
        _header->GetLabels()[2]->setText("y");
        _header->SetChToolTip(2, _STYLE_TOOLTIP_("Quaternion y component"));
        _header->GetLabels()[3]->setText("z");
        _header->SetChToolTip(3, _STYLE_TOOLTIP_("Quaternion z component"));
    }

    //  Register with the mux
    DataMultiplexer::GetI().RegisterGraph(this->objectName(), _nInputs, this);
}

/**
 * @brief [Slot] Handle switching between the plot modes (euler vs quat.)
 * @param rpySelected 'true' if euler mode has been selected
 */
void OrientationWindow::InputTypeUpdated(bool rpySelected)
{
    emit logLine("3D: Mode change requested");
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
 *      changed in the drop-down fields of the header. It updates the channels
 *      used as data sources for the plot.
 * @param inChannels    Array of 3 input channel indexes
 */
void OrientationWindow::UpdateInputChannels(uint8_t *inChannels)
{
    emit logLine("3D: Updating input channels");
   _inputChannels[0] = inChannels[0];
   _inputChannels[1] = inChannels[1];
   _inputChannels[2] = inChannels[2];

  _maxInChannel = 0;
  for (uint8_t i = 0; i < 3; i++)
      if (inChannels[i] > _maxInChannel)
          _maxInChannel = inChannels[i];
}
