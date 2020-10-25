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

#include <helperObjects/dataMultiplexer/datamultiplexer.h>
#include <mainwindow.h>

using namespace QtDataVisualization;

const int lowerNumberOfItems = 10000;
const float lowerCurveDivider = 0.75f;

ScatterDataModifier::ScatterDataModifier(Q3DScatter *scatter)
    : m_fontSize(40.0f),
      m_style(QAbstract3DSeries::MeshPoint),
      m_itemCount(lowerNumberOfItems),
      m_curveDivider(lowerCurveDivider)
{
    m_graph = scatter;

    //  Create container window and set size policy
    _contWind = new QWidget();

    //  Main vertical layout
    QVBoxLayout *windMainLayout = new QVBoxLayout(_contWind);
    //this->setLayout(windMainLayout);
    this->setWidget(_contWind);


    header = new graphHeaderWidget(3, false);
    windMainLayout->addLayout(header->GetLayout());

    QWidget *graphCont = QWidget::createWindowContainer(m_graph);
    graphCont->setMinimumSize(QSize(200,200));
    graphCont->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    windMainLayout->addWidget(graphCont,1);


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

    QScatterDataProxy *proxy = new QScatterDataProxy(this);
    QScatter3DSeries *series = new QScatter3DSeries(proxy, this);
    series->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
    series->setMeshSmooth(true);
    m_graph->addSeries(series);
    //! [2]
    if (m_graph->seriesList().size())
        m_graph->seriesList().at(0)->setMesh(m_style);
    m_graph->activeTheme()->setBackgroundEnabled(false);
    changeFont(QFont("Arial"));

    _dataArray = new QScatterDataArray();
    _dataArray->resize(m_itemCount);

    m_graph->seriesList().at(0)->dataProxy()->resetArray(_dataArray);
}

ScatterDataModifier::~ScatterDataModifier()
{
    qDebug() << "Destroying the scatter plot";
    //m_graph->deleteLater();
    DataMultiplexer::GetI().UnregisterGraph(this);

   qDebug() << "Unregistered graph, disconnecting from mux";

   QObject::disconnect(DataMultiplexer::GetP(),
                    &DataMultiplexer::ChannelsUpdated,
                    header,
                    &graphHeaderWidget::UpdateChannelDropdown);

   QObject::disconnect(header, &graphHeaderWidget::UpdateInputChannels,
                    this, &ScatterDataModifier::UpdateInputChannels);
   delete header;

   m_graph->seriesList().clear();
   m_graph->close();
   m_graph->~Q3DScatter();

   qDebug() << "Deleted UI";
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

/**
 * @brief Handle closing of this window
 *  Instead of closing the window, simply hide it. This leaks memory but is the
 *  only way to remove the window with crashing the app. (every time a
 *  deconstructor is called on 3Dscatter plot, we get segmentation fault
 * @param closeEvent
 */
//void ScatterDataModifier::closeEvent(QCloseEvent *closeEvent)
//{
//    qDebug()<<"Closing: "<<closeEvent;

//    qDebug() << "Destroying the scatter plot";
//    //m_graph->deleteLater();
//    DataMultiplexer::GetI().UnregisterGraph(this);
//   qDebug() << "Unregistered graph, disconnecting from mux";
//   QObject::disconnect(DataMultiplexer::GetP(),
//                    &DataMultiplexer::ChannelsUpdated,
//                    header,
//                    &graphHeaderWidget::UpdateChannelDropdown);

//   QObject::disconnect(header, &graphHeaderWidget::UpdateInputChannels,
//                    this, &ScatterDataModifier::UpdateInputChannels);
//   delete header;

//   //m_graph->seriesList().clear();
//   //m_graph->close();
//   //m_graph->deleteLater();
//   this->deleteLater();

//    //this->hide();
//    //closeEvent->ignore();
//}

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

