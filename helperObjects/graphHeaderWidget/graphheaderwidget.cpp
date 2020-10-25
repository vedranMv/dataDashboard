#include "graphheaderwidget.h"
#include "helperObjects/dataMultiplexer/datamultiplexer.h"
#include <mainwindow.h>

#include <QWidget>
#include <QLabel>
#include <QObject>
#include <QPushButton>

graphHeaderWidget::graphHeaderWidget(uint8_t chnnelNum, bool hasBoundaries)
    : _bounded(hasBoundaries), _parent(nullptr)
{
    _controlLayout = new QHBoxLayout();
    QVBoxLayout *chLabels = new QVBoxLayout();
    QVBoxLayout *inChannels = new QVBoxLayout();
    QSpacerItem *horSpacer = new QSpacerItem (20,20,QSizePolicy::Expanding);

    _controlLayout->addLayout(chLabels);
    _controlLayout->addLayout(inChannels);
    _controlLayout->addSpacerItem(horSpacer);

    _inputChannelList = new uint8_t[chnnelNum];

    for (uint8_t i = 0; i < chnnelNum; i++)
    {
        QLabel *chLab = new QLabel();
        chLab->setText("Channel "+QString::number(i));

        inCh.push_back(new QComboBox());
        //
        QObject::connect(inCh[i],
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &graphHeaderWidget::ComboBoxUpdated);

        chLabels->addWidget(chLab);
        inChannels->addWidget(inCh.back());
    }
    UpdateChannelDropdown();

}

void graphHeaderWidget::UpdateChannelDropdown()
{
    for (QComboBox* X : inCh)
    {
        //  Save selected entry in an attempt to reselect it after refresh
        QString currentItem = X->currentText();
        //  Clear list
        X->clear();
        //  Insert new list
        X->addItems(DataMultiplexer::GetI().GetChannelList());
        //  Attempt to reselect old value
        int newIndex = X->findText(currentItem);
        if (newIndex >= 0)
            X->setCurrentIndex(newIndex);
    }
}

QStringList graphHeaderWidget::GetChannelLabels()
{
    QStringList retVal;

    for (QComboBox* X : inCh)
        retVal.append(X->currentText());

    return retVal;
}


void graphHeaderWidget::ComboBoxUpdated(const int &)
{
//    uint8_t inputChannelList[3];

//    for (uint8_t i = 0; i < 3; i++)
//        inputChannelList[i] = inCh[i]->currentIndex();

//    emit UpdateInputChannels(inputChannelList);
    for (uint8_t i = 0; i < 3; i++)
        _inputChannelList[i] = inCh[i]->currentIndex();

    emit UpdateInputChannels(_inputChannelList);
}

graphHeaderWidget::~graphHeaderWidget()
{
    _controlLayout->deleteLater();
    delete [] _inputChannelList;
}
