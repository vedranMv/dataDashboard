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

#include "scatterdatamodifier.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtWidgets/QComboBox>
#include <QVBoxLayout>

#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>

using namespace QtDataVisualization;

const int lowerNumberOfItems = 10000;
const float lowerCurveDivider = 0.75f;

ScatterDataModifier::ScatterDataModifier()
    : m_fontSize(40.0f),
      m_style(QAbstract3DSeries::MeshPoint),
      m_itemCount(lowerNumberOfItems),
      m_curveDivider(lowerCurveDivider)
{
    m_graph = new Q3DScatter();

    //  Create container window and set size policy
    _contWind = QWidget::createWindowContainer(m_graph);
    _contWind->setMinimumSize(QSize(200,200));
    _contWind->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //  Make sure to delete the object when underlying window is deleted
    connect(_contWind, &QWidget::destroyed, this, &ScatterDataModifier::deleteLater);

    QVBoxLayout *windMainLayout = new QVBoxLayout();
    this->setLayout(windMainLayout);

    graphHeaderWidget *header = new graphHeaderWidget(3, false);
    windMainLayout->addLayout(header->GetLayout());
    windMainLayout->addWidget(_contWind);

    //  Handle dynamic channel selection by dropdown
    QObject::connect(header, &graphHeaderWidget::UpdateInputChannels,
                     this, &ScatterDataModifier::UpdateInputChannels);

    //  Make sure input channel dropdowns have updated list of channels
    QObject::connect(DataMultiplexer::GetP(),
                     &DataMultiplexer::ChannelsUpdated,
                     header,
                     &graphHeaderWidget::UpdateChannelDropdown);

    //! [0]
    m_graph->activeTheme()->setType(Q3DTheme::ThemeEbony);
    QFont font = m_graph->activeTheme()->font();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setFont(font);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualitySoftLow);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    //! [0]

    //! [2]
    QScatterDataProxy *proxy = new QScatterDataProxy;
    QScatter3DSeries *series = new QScatter3DSeries(proxy);
    series->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    series->setMeshSmooth(true);
    m_graph->addSeries(series);
    //! [2]
    if (m_graph->seriesList().size())
        m_graph->seriesList().at(0)->setMesh(m_style);
    m_graph->activeTheme()->setBackgroundEnabled(false);
    changeFont(QFont("Arial"));

    _dataArray = new QScatterDataArray;
    _dataArray->resize(m_itemCount);

    m_graph->seriesList().at(0)->dataProxy()->resetArray(_dataArray);
}

ScatterDataModifier::~ScatterDataModifier()
{
    qDebug() << "Destroying the scatter plot";

    DataMultiplexer::GetI().UnregisterGraph(this);
    //delete _dataArray;
    //delete m_graph;
}

/**
 * @brief [Slot] Function that is called whenever input channel has been
 *      changed in the dropdown fields of the header. It updates the channel
 *      selection stored in this object.
 * @param inChannels    Array of 3 input channel indexes
 */
void ScatterDataModifier::UpdateInputChannels(uint8_t *inChannels)
{
   _inputChannels[0] = inChannels[0];
   _inputChannels[1] = inChannels[1];
   _inputChannels[2] = inChannels[2];

  _maxInChannel = 0;
  for (uint8_t i = 0; i < 3; i++)
      if (inChannels[i] > _maxInChannel)
          _maxInChannel = inChannels[i];
}

/**
 * @brief Function directly called by the multiplexer to push data into
 *      the graph
 * @param data  Array of available data
 * @param n     Size of data array
 */
void ScatterDataModifier::ReceiveData(double *data, uint8_t n)
{
    static uint32_t index = 0;
    // Check if the largest index of input channels is available in the
    // received block of data
    if (n < _maxInChannel)
        return;

    m_graph->seriesList().at(0)->dataProxy()->insertItem(index,QScatterDataItem(QVector3D( (float)data[ _inputChannels[1] ],
                                                         (float)data[ _inputChannels[2] ],
                                                         (float)data[ _inputChannels[0] ])));
    index = (index+1) % lowerNumberOfItems;
}

void ScatterDataModifier::addData()
{
    // Configure the axes according to the data
    //! [4]
    m_graph->axisX()->setTitle("X");
    m_graph->axisY()->setTitle("Y");
    m_graph->axisZ()->setTitle("Z");
    //! [4]

    //! [5]
    QScatterDataArray *dataArray = new QScatterDataArray;
    dataArray->resize(m_itemCount);
    QScatterDataItem *ptrToDataArray = &dataArray->first();
    //! [5]


    for (int i = 0; i < m_itemCount; i++) {
        //ptrToDataArray->setPosition(randVector());
        ptrToDataArray++;
    }

    //! [7]
    m_graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);

    //! [7]
}

//! [8]
void ScatterDataModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_style = QAbstract3DSeries::Mesh(comboBox->itemData(style).toInt());
        if (m_graph->seriesList().size())
            m_graph->seriesList().at(0)->setMesh(m_style);
    }
}

void ScatterDataModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    newFont.setPointSizeF(m_fontSize);
    m_graph->activeTheme()->setFont(newFont);
}

void ScatterDataModifier::setBackgroundEnabled(int enabled)
{
    m_graph->activeTheme()->setBackgroundEnabled((bool)enabled);
}

void ScatterDataModifier::setGridEnabled(int enabled)
{
    m_graph->activeTheme()->setGridEnabled((bool)enabled);
}
//! [8]

