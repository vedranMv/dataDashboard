#include "lineplot.h"
#include "helperObjects/dataMultiplexer/datamultiplexer.h"

const uint32_t dataLen = 500;
LinePlot::LinePlot()
{
     _contWind = new QWidget();

     //  Main vertical layout
     QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);
     //this->setLayout(windMainLayout);
     this->setWidget(_contWind);

     _header = new graphHeaderWidget(2, false);
     windMainLayout->addLayout(_header->GetLayout());
     _header->AppendHorSpacer();

     //  Handle dynamic channel selection by dropdown
     QObject::connect(_header, &graphHeaderWidget::UpdateInputChannels,
                      this, &LinePlot::UpdateInputChannels);

     //  Make sure input channel dropdowns have updated list of channels
     QObject::connect(DataMultiplexer::GetP(),
                      &DataMultiplexer::ChannelsUpdated,
                      _header,
                      &graphHeaderWidget::UpdateChannelDropdown);

    _plot = new QCustomPlot();
    _plot->addGraph();
    _plot->graph()->setPen(QPen(Qt::blue));
    _plot->graph()->setBrush(QBrush(QColor(0, 0, 255, 20)));
    _plot->addGraph();
    _plot->graph()->setPen(QPen(Qt::red));
    _plot->axisRect()->setupFullAxesBox(true);
    _plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    _plot->xAxis->setRange(-500, 0);
    _plot->yAxis->setRange(-1, 1);

    _inputCh[0].resize(dataLen);
    _inputCh[1].resize(dataLen);
    _xAxis.resize(dataLen);
    for (int32_t i = 0; i < dataLen; i++)
        _xAxis[i] = (double)i-(double)dataLen;
    windMainLayout->addWidget(_plot);

    _refresher = new QTimer(this);
    connect(_refresher, SIGNAL(timeout()), _plot, SLOT(replot()));
    _refresher->start(20);
}

LinePlot::~LinePlot()
{
    qDebug() << "Destroying the line plot";
    DataMultiplexer::GetI().UnregisterGraph(this);

}

void LinePlot::refresh()
{
    _plot->repaint();
}

/**
 * @brief [Slot] Function that is called whenever input channel has been
 *      changed in the dropdown fields of the header. It updates the channel
 *      selection stored in this object.
 * @param inChannels    Array of 3 input channel indexes
 */
void LinePlot::UpdateInputChannels(uint8_t *inChannels)
{
   _inputChannels[0] = inChannels[0];
   _inputChannels[1] = inChannels[1];
   //_inputChannels[2] = inChannels[2];

  _maxInChannel = 0;
  for (uint8_t i = 0; i < 2; i++)
      if (inChannels[i] > _maxInChannel)
          _maxInChannel = inChannels[i];

  QStringList chLabels = _header->GetChannelLabels();
  _plot->graph(0)->setName(chLabels[0]);
  _plot->graph(1)->setName(chLabels[1]);
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void LinePlot::ReceiveData(double *data, uint8_t n)
{
    static uint32_t index = 0;
    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    _inputCh[0][ index ] = data[_inputChannels[0]];
    _inputCh[1][ index ] = data[_inputChannels[1]];

    _plot->graph(0)->setData(_xAxis, _inputCh[0]);
    _plot->graph(1)->setData(_xAxis, _inputCh[1]);

    if (_inputCh[0][ index ] < _plot->yAxis->range().lower)
        _plot->yAxis->setRange(_inputCh[0][ index ]*1.1, _plot->yAxis->range().upper);
    if (_inputCh[0][ index ] > _plot->yAxis->range().upper)
        _plot->yAxis->setRange(_plot->yAxis->range().lower, _inputCh[0][ index ]*1.1);

    if (_inputCh[1][ index ] < _plot->yAxis->range().lower)
        _plot->yAxis->setRange(_inputCh[1][ index ]*1.1, _plot->yAxis->range().upper);
    if (_inputCh[1][ index ] > _plot->yAxis->range().upper)
        _plot->yAxis->setRange(_plot->yAxis->range().lower, _inputCh[1][ index ]*1.1);

    //_plot->replot();

    // Add data to the plot
    index = (index+1) % dataLen;
}
