/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "scatterwindow.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtWidgets/QComboBox>
#include <QLabel>
#include <QString>

#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <mainwindow.h>

using namespace QtDataVisualization;

const int dataSize = 10000;

ScatterWindow::ScatterWindow(QString objName): _plotDataSize(dataSize)
{
    _graph = new Q3DScatter();
    this->setObjectName(objName);
    //  Create container window and set size policy
    _contWind = new QWidget();

    //  Main vertical layout
    QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);
    this->setWidget(_contWind);

    _header = new graphHeaderWidget(3, this->objectName());
    windMainLayout->addLayout(_header->GetLayout());
    //  Update channel labels
    _header->GetLabels()[0]->setText("X-axis");
    _header->SetChToolTip(0, \
                    _STYLE_TOOLTIP_("Select X-axis of the scatter plot"));
    _header->GetLabels()[1]->setText("Y-axis");
    _header->SetChToolTip(1, \
                    _STYLE_TOOLTIP_("Select Y-axis of the scatter plot"));
    _header->GetLabels()[2]->setText("Z-axis");
    _header->SetChToolTip(2, \
                    _STYLE_TOOLTIP_("Select Z-axis of the scatter plot"));

    //  Line to separate channels from config
    QFrame *_vertLine = new QFrame();
    _vertLine->setFrameShape(QFrame::VLine);
    _vertLine->setFrameShadow(QFrame::Sunken);
    _header->GetLayout()->addWidget(_vertLine);

    //  Header items for orientation plot
    QVBoxLayout *scatterSpecificHeader = new QVBoxLayout();
    _header->GetLayout()->addLayout(scatterSpecificHeader);
    _header->AppendHorSpacer();

    //  Data size line edit
    scatterSpecificHeader->addWidget(new QLabel("Data size"));
    QLineEdit *_plotDataSizeLE = new QLineEdit();
    _plotDataSizeLE->setValidator( new QIntValidator(1, _plotDataSize*10, this) );
    _plotDataSizeLE->setToolTip(_STYLE_TOOLTIP_("Number of past data points "
                            "kept in the graph (automatically updated)"));
    _plotDataSizeLE->setText(QString::number(_plotDataSize));

    QObject::connect(_plotDataSizeLE, &QLineEdit::textChanged,
                     this, &ScatterWindow::on_dataSize_changed);
    scatterSpecificHeader->addWidget(_plotDataSizeLE);
    //  Reset data set push button
    QPushButton *resetView = new QPushButton();
    resetView->setText("Clear data");
    resetView->setToolTip(_STYLE_TOOLTIP_("Press to clear all data from the"
                                          " scatter plot"));
    QObject::connect(resetView, &QPushButton::pressed,
                     this, &ScatterWindow::on_resetData_pressed);
    scatterSpecificHeader->addWidget(resetView);


    QWidget *graphCont = QWidget::createWindowContainer(_graph);
    graphCont->setMinimumSize(QSize(200,200));
    graphCont->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    windMainLayout->addWidget(graphCont,1);


    //  Handle dynamic channel selection by drop-down
    QObject::connect(_header, &graphHeaderWidget::UpdateInputChannels,
                     this, &ScatterWindow::UpdateInputChannels);

    //  Make sure input channel drop-downs have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     _header,
                     &graphHeaderWidget::UpdateChannelDropdown);

    //  Stylize the graph
    _graph->activeTheme()->setType(Q3DTheme::ThemeEbony);
    QFont font = _graph->activeTheme()->font();
    font.setPointSize(40.0f);
    _graph->activeTheme()->setFont(font);
    _graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    _graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

    QScatterDataProxy *proxy = new QScatterDataProxy(this);
    QScatter3DSeries *series = new QScatter3DSeries(proxy, this);
    series->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    series->setMeshSmooth(true);

    //  Add data series we'll be working with
    _graph->addSeries(series);
    if (_graph->seriesList().size())
        _graph->seriesList().at(0)->setMesh(QAbstract3DSeries::MeshPoint);
    _graph->activeTheme()->setBackgroundEnabled(false);
    changeFont(QFont("Arial"));

    _dataArray = new QScatterDataArray();
    _dataArray->resize(_plotDataSize);

    _graph->seriesList().at(0)->dataProxy()->resetArray(_dataArray);

    //  Register with the mux
    DataMultiplexer::GetI().RegisterGraph(this->objectName(), 3, this);
}

/**
 * @brief ScatterWindow::~ScatterWindow
 */
ScatterWindow::~ScatterWindow()
{
    emit logLine("Scatter: Destroying the plot");
    DataMultiplexer::GetI().UnregisterGraph(this);

    QObject::disconnect(DataMultiplexer::GetP(),
                    &DataMultiplexer::ChannelsUpdated,
                    _header,
                    &graphHeaderWidget::UpdateChannelDropdown);

    QObject::disconnect(_header, &graphHeaderWidget::UpdateInputChannels,
                    this, &ScatterWindow::UpdateInputChannels);
    delete _header;

    _graph->seriesList().clear();
    _graph->close();
    delete _graph;

    MainWindow::clearLayout(_contWind->layout());
}

/**
 * @brief [Slot] Function that is called whenever input channel has been
 *      changed in the drop-down fields of the header. It updates the channels
 *      used as data sources for the plot.
 * @param inChannels    Array of 3 input channel indexes
 */
void ScatterWindow::UpdateInputChannels(uint8_t *inChannels)
{
    emit logLine("Scatter: Updating input channels");
    _inputChannels[0] = inChannels[0];
    _inputChannels[1] = inChannels[1];
    _inputChannels[2] = inChannels[2];

    _maxInChannel = 0;
    for (uint8_t i = 0; i < 3; i++)
        if (inChannels[i] > _maxInChannel)
            _maxInChannel = inChannels[i];

    _graph->axisX()->setTitle(_header->GetChLabels()[0]);
    _graph->axisY()->setTitle(_header->GetChLabels()[1]);
    _graph->axisZ()->setTitle(_header->GetChLabels()[2]);

    //  Reset plot on channel change
    on_resetData_pressed();
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void ScatterWindow::ReceiveData(double *data, uint8_t n)
{
    static uint32_t index = 0;
    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    _graph->seriesList().at(0)->dataProxy()->setItem(index,QScatterDataItem(QVector3D( (float)data[ _inputChannels[0] ],
                                                         (float)data[ _inputChannels[1] ],
                                                         (float)data[ _inputChannels[2] ])));
    index = (index+1) % _plotDataSize;
}


void ScatterWindow::changeFont(const QFont &font)
{
    QFont newFont = font;
    newFont.setPointSizeF(40.0f);
    _graph->activeTheme()->setFont(newFont);
}


void ScatterWindow::setGridEnabled(int enabled)
{
    _graph->activeTheme()->setGridEnabled((bool)enabled);
}

/**
 * @brief [Slot] Handles change in data size line edit
 * @param __plotDataSize Current text of line edit
 */
void ScatterWindow::on_dataSize_changed(const QString &__plotDataSize)
{
    //  Safe to assign, validation rule defined in the constructor
    _plotDataSize = __plotDataSize.toInt();

    _dataArray->resize(_plotDataSize);
    _graph->seriesList().at(0)->dataProxy()->resetArray(_dataArray);
}

/**
 * @brief [Slot] Handles press of a 'Reset data button'
 */
void ScatterWindow::on_resetData_pressed()
{
    emit logLine("Scatter: Data reset requested");
    _dataArray->clear();
    _dataArray->resize(_plotDataSize);
    _graph->seriesList().at(0)->dataProxy()->resetArray(_dataArray);
}
