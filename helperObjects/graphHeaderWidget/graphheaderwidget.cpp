#include "graphheaderwidget.h"
#include "helperObjects/dataMultiplexer/datamultiplexer.h"
#include <mainwindow.h>

#include <QWidget>
#include <QLabel>
#include <QObject>
#include <QPushButton>

/**
 * @brief Create header widget
 *      Header widget contains main layout (horiz.) and two vertical
 *      widgets inside of it containing labels and combo-boxes
 *  1111111111111111
 *  1 2222 3333 * /1
 *  1 2  2 3  3 * /1
 *  1 2222 3333 * /1
 *  1111111111111111
 *      Stars depict area where extra horizontal widgets can be injected
 *      before \ref AppendHorSpacer is called. Finally, horizontal spacer
 *      is added in the back, shown in /
 * @param chnnelNum Number of input channels
 */
graphHeaderWidget::graphHeaderWidget(uint8_t chnnelNum)
    :  _parent(nullptr)
{
    //  Create layouts
    _mainLayout = new QHBoxLayout();
    QVBoxLayout *chLabels = new QVBoxLayout();
    QVBoxLayout *inChannels = new QVBoxLayout();

    _mainLayout->addLayout(chLabels);
    _mainLayout->addLayout(inChannels);

    _inputChannelList = new uint8_t[chnnelNum];

    //  Loop to create channel labels and drop-downs
    for (uint8_t i = 0; i < chnnelNum; i++)
    {
        _inChLabel.push_back(new QLabel());
        _inChLabel.back()->setText("Channel "+QString::number(i));

        _inCh.push_back(new QComboBox());
        //  Set fixed height for nicer look
        _inCh.back()->setFixedHeight(22);
        _inChLabel.back()->setFixedHeight(22);
        //  Trigger 'ComboBoxUpdated' function here whenever the combo-boxes
        //  get updated
        QObject::connect(_inCh[i],
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &graphHeaderWidget::ComboBoxUpdated);

        chLabels->addWidget(_inChLabel.back());
        inChannels->addWidget(_inCh.back());
    }
    UpdateChannelDropdown();
}

/**
 * @brief Add horizontal spacer to the end of the layout
 *      Seals the header widget once the header contains all the
 *      necessary fields
 */
void graphHeaderWidget::AppendHorSpacer()
{
    _mainLayout->addSpacerItem(new QSpacerItem (20,20,QSizePolicy::Expanding));
}

/**
 * @brief [Slot] Called from mux to refresh the list of channels in the
 *  drop-down menus
 */
void graphHeaderWidget::UpdateChannelDropdown()
{
    for (QComboBox* X : _inCh)
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

/**
 * @brief Returns reference to labels so they can be stylize or edited from
 *      external sources
 * @return Reference to the vector of labels
 */
QVector<QLabel*>& graphHeaderWidget::GetLabels()
{
    return _inChLabel;
};

/**
 * @brief Returns main header layout
 * @return main header layout
 */
QHBoxLayout* graphHeaderWidget::GetLayout()
{
    return _mainLayout;
}

/**
 * @brief [Private Slot] Handles pushing an updated list of input channels to
 *      the parent window
 *  Called by the combo-boxes when their value is changed
 */
void graphHeaderWidget::ComboBoxUpdated(const int &)
{
    for (uint8_t i = 0; i < _inCh.size(); i++)
        _inputChannelList[i] = _inCh[i]->currentIndex();

    emit UpdateInputChannels(_inputChannelList);
}

graphHeaderWidget::~graphHeaderWidget()
{
    delete [] _inputChannelList;
}
