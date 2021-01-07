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

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>
#include <QWidget>
#include <QVBoxLayout>
#include <QMdiSubWindow>

#include <helperObjects/graphHeaderWidget/graphheaderwidget.h>


using namespace QtDataVisualization;

class ScatterWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    explicit ScatterWindow(QString objName="");
    ~ScatterWindow() override;

    void UpdateInputChannels(uint8_t *inChannels);
    void ReceiveData(double *data, uint8_t n);

    void changeFont(const QFont &font);
    void setGridEnabled(int enabled);

signals:
    void logLine(const QString &line);

public slots:
    void on_dataSize_changed(const QString &datasize);
    void on_resetData_pressed();

private:
    QWidget *_contWind;
    Q3DScatter *_graph;
    QScatterDataArray *_dataArray;

    uint8_t _inputChannels[3];
    uint8_t _maxInChannel;

    graphHeaderWidget *_header;
};

#endif
