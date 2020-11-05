#include "lineplot.h"
#include "helperObjects/dataMultiplexer/datamultiplexer.h"
#include <mainwindow.h>


uint32_t _XaxisSize = 500;
LinePlot::LinePlot():  _nInputs(1), _maxInChannel(0), _index(0)
{
     //  Create container window
    _contWind = new QWidget();
    windMainLayout = new QVBoxLayout(_contWind);
    this->setWidget(_contWind);

    _refresher = new QTimer(this);

    _ConstructUI();

    _plotDataMutex.release();
}

void LinePlot::_ConstructUI()
{
    //  Check if UI is already been constructed, then destroy it
    if (!_contWind->layout()->isEmpty())
    {
        _refresher->stop();
        DataMultiplexer::GetI().UnregisterGraph(this);
        disconnect(_refresher, SIGNAL(timeout()), _plot, SLOT(replot()));
        disconnect(DataMultiplexer::GetP(),
                         &DataMultiplexer::ChannelsUpdated,
                         _header,
                         &graphHeaderWidget::UpdateChannelDropdown);
        MainWindow::clearLayout(_contWind->layout());
    }

    //  Basic header with input channel drop-downs
    _header = new graphHeaderWidget(_nInputs);
    windMainLayout->addLayout(_header->GetLayout());


    //  Handle dynamic channel selection by drop-down
    QObject::connect(_header, &graphHeaderWidget::UpdateInputChannels,
                     this, &LinePlot::UpdateInputChannels);

    //  Make sure input channel drop-downs have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     _header,
                     &graphHeaderWidget::UpdateChannelDropdown);
    //  Extra parts of header, specific to Line plot
    QVBoxLayout *lineSpecificHeader = new QVBoxLayout();
    _header->GetLayout()->addLayout(lineSpecificHeader);
    _header->AppendHorSpacer();

    //  'Add plot' button and add it to the layout
    QPushButton *addPlot = new QPushButton();
    addPlot->setText("Add plot");
    connect(addPlot, SIGNAL(pressed()), this, SLOT(ChannelAdded()));
    lineSpecificHeader->addWidget(addPlot);

    //  Checkbox to toggle automatic scaling on y-axis
    _autoAdjustYaxis = new QCheckBox();
    _autoAdjustYaxis->setText("Auto Y scale");
    _autoAdjustYaxis->setChecked(true);
    lineSpecificHeader->addWidget(_autoAdjustYaxis);

    //  Textbox to update the size of x-axis
    QLineEdit *xAxisSize = new QLineEdit();
    xAxisSize->setValidator( new QIntValidator(10, 5000, this) );
    xAxisSize->setText( QString::number(_XaxisSize) );
    connect(xAxisSize, &QLineEdit::textChanged, this, &LinePlot::UpdateXaxis);
    lineSpecificHeader->addWidget(new QLabel("X axis size"));
    lineSpecificHeader->addWidget(xAxisSize);

    lineSpecificHeader->addSpacerItem(new QSpacerItem (20,20,QSizePolicy::Expanding));

    // Create plot
    _plot = new QCustomPlot();
    _plot->setMinimumSize(QSize(200,200));
    _plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(_refresher, SIGNAL(timeout()), _plot, SLOT(replot()));

    if (_inputCh.size() < _nInputs)
        _inputCh.resize(_nInputs);

    //   Based on the number of inputs, add graphs into the plot
    for (uint8_t i = 0; i < _nInputs; i++)
    {
        _plot->addGraph();
        //   There's 19 predefined colours, make sure we assign a unique colour
        //   to a channel, but never overflow
        _plot->graph()->setPen(QPen((Qt::GlobalColor)((i+6) % 19)));

        //  Match label of the channel to the graph colour
        QPalette palette = _header->GetLabels()[i]->palette();
        palette.setColor(_header->GetLabels()[i]->foregroundRole(), (Qt::GlobalColor)((i+6) % 19));
        _header->GetLabels()[i]->setPalette(palette);

        _inputCh[i].resize(_XaxisSize);
    }
    _index = 0;
    _inputChannels.resize(_nInputs);

    //  General graph configuration, regardless of individual graphs
   _plot->axisRect()->setupFullAxesBox(true);
   _plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
   _plot->axisRect()->setRangeZoom(Qt::Vertical);
   _plot->axisRect()->setRangeDrag(Qt::Vertical);
   _plot->xAxis->setRange(-(double)_XaxisSize, 0);
   _plot->yAxis->setRange(-1, 1);

   //   Populate X-axis values
   _xAxis.resize(_XaxisSize);
   for (uint32_t i = 0; i < _XaxisSize; i++)
       _xAxis[i] = (double)i - (double)_XaxisSize;

   //   Plot is configured, add to the window
   windMainLayout->addWidget(_plot);

    //  Restart the timer for refreshing UI
    _refresher->start(20);
    qDebug()<<"Updated size to: "<<_XaxisSize;

   //  Register with the mux
   DataMultiplexer::GetI().RegisterGraph(this->objectName(), _nInputs, this);
}


LinePlot::~LinePlot()
{
    emit logLine("Destroying the line plot");
    DataMultiplexer::GetI().UnregisterGraph(this);
    //  Wait to get mutex before deleting the rest. Prevents rare crashes
    //  when closing the window
    _plotDataMutex.acquire();
    _plotDataMutex.release();
}

void LinePlot::ChannelAdded()
{
    _plotDataMutex.acquire();

    _nInputs++;
    _ConstructUI();

    _plotDataMutex.release();
}

/**
 * @brief [Slot] Adjust length of X axis in UI based on user input
 *      Called when textbox in UI is edited. Changes the size of X axis
 * @param _dataSize
 */
void LinePlot::UpdateXaxis(const QString &_dataSize)
{
    _plotDataMutex.acquire();
qDebug()<<"Updated size to: "<<_XaxisSize;
    _XaxisSize = _dataSize.toUInt();

    //  Adjust plot range
    _plot->xAxis->setRange(-(double)_XaxisSize, 0);

    //   Populate X-axis vector
    _xAxis.resize(_XaxisSize);
    for (uint32_t i = 0; i < _XaxisSize; i++)
        _xAxis[i] = (double)i - (double)_XaxisSize;

    //   Resize input arrays
    for (uint8_t i = 0; i < _nInputs; i++)
    {
        _inputCh[i].clear();
        _inputCh[i].resize(_XaxisSize);
    }
    //  Reset current index of input arrays
    _index = 0;

    _plotDataMutex.release();
}
/**
 * @brief [Slot] Function that is called whenever input channel has been
 *      changed in the drop-down fields of the header. It updates the channels
 *      used as data sources for the plot.
 * @param inChannels    Array of \ref _nInputs input channel indexes
 */
void LinePlot::UpdateInputChannels(uint8_t *inChannels)
{
    //  TODO: implement error reporting
    if (_header->GetLabels().size() != _nInputs)
    {
        emit logLine("Line plot: inconsistency in received and required channels.");
        return;
    }

    _plotDataMutex.acquire();

    //  Reset the max index of input channels
    _maxInChannel = 0;

    for (uint8_t i = 0; i < _nInputs; i++)
    {
        _inputChannels[i] = inChannels[i];

        if (inChannels[i] > _maxInChannel)
            _maxInChannel = inChannels[i];

        _plot->graph(i)->setName(_header->GetLabels()[i]->text());
    }

    _plotDataMutex.release();
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void LinePlot::ReceiveData(double *data, uint8_t n)
{
    double maxYVal, minYVal;

    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    //  Initialize Y axis limits
    maxYVal = minYVal = data[_inputChannels[0]];

    //  If we can't acquire mutex in a millisecond, return and try again
    //  with new data
    //  TODO: can it be problematic in low data-rate applications if we arrive
    //  here while another element holds the mutex?
    if (!_plotDataMutex.tryAcquire(1,1))
        return;

    for (uint8_t i = 0; i < _nInputs; i++)
    {
        //  Populate the graph
        if ((_index+1) != _XaxisSize)
        {
            //  If we have less then max number of data points, keep adding
            //  to the array from the back. Visually, graph comes into existence
            //  from right, then moving towards left
            _inputCh[i][_XaxisSize - 1 - _index] = data[_inputChannels[i]];
        }
        else
        {
            //  Once we have filled up data array, we keep popping out the
            //  oldest data point, and pushing in the new. Visually, the
            //  direction of movement as the graph above
            _inputCh[i].pop_front();
            _inputCh[i].push_back(data[_inputChannels[i]]);
        }

        //  Connect updated data source to graph
        _plot->graph(i)->setData(_xAxis, _inputCh[i]);

        //  Update Y axis limits boundaries
        if (data[_inputChannels[i]] > maxYVal) maxYVal = data[_inputChannels[i]];
        if (data[_inputChannels[i]] < minYVal) minYVal = data[_inputChannels[i]];
    }

    //  If enabled, automatically adjust range to 10% more than max value, and
    //  10% less then min value
    if (_autoAdjustYaxis->isChecked())
    {
        if (minYVal < _plot->yAxis->range().lower)
            _plot->yAxis->setRange(minYVal * 1.1, _plot->yAxis->range().upper);
        if (maxYVal > _plot->yAxis->range().upper)
            _plot->yAxis->setRange(_plot->yAxis->range().lower, maxYVal * 1.1);
    }

    _plotDataMutex.release();

    // Increment data index and enforce the boundary
    if ((_index+1) != _XaxisSize)
        _index = (_index+1) % _XaxisSize;
}
