#include "mathchannelcomponent.h"
#include "helperObjects/dataMultiplexer/mathchannel.h"

#define _STYLE_TOOLTIP_(X) "<html><head/><body><p>" X "</p></body></html>"


UIMathChannelComponent::~UIMathChannelComponent()
{
    //  Deletion of object is called from MainWindow
    //  Here we only need to delete the layout
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        delete item;
    }
    layout->deleteLater();
}

/**
 * @brief Constructor
 * @param id unique identifier of the component
 */
UIMathChannelComponent::UIMathChannelComponent(uint8_t id): _id(id)
{
    QLabel *labels[4];
    QSpacerItem *horSpacer;

    labels[0] = new QLabel("Math channel");
    labels[1] = new QLabel("+= ");
    labels[2] = new QLabel("(Input channel");
    labels[3] = new QLabel(")");

    mathChSelector = new QComboBox();
    inChSelector = new QSpinBox();

    //  Populate dropdown with operation text
    mathSelector = new QComboBox();
    for (auto &X : MathOperationText)
        mathSelector->addItem(X);

    deleteButton = new QPushButton();
    deleteButton->setText("X");
    deleteButton->setFixedWidth(25);

    layout = new QHBoxLayout();
    layout->setObjectName("mathChannel_"+QString::number(_id));

    horSpacer = new QSpacerItem (20,20,QSizePolicy::Expanding);

    //  Construct layout with all the elements
    layout->addWidget(labels[0], 0, Qt::AlignLeft);
    layout->addWidget(mathChSelector, 0, Qt::AlignLeft);
    layout->addWidget(labels[1], 0, Qt::AlignLeft);
    layout->addWidget(mathSelector, 0, Qt::AlignLeft);
    layout->addWidget(labels[2], 0, Qt::AlignLeft);
    layout->addWidget(inChSelector, 0, Qt::AlignLeft);
    layout->addWidget(labels[3], 0, Qt::AlignLeft);

    layout->addSpacerItem(horSpacer);
    layout->addWidget(deleteButton, 0, Qt::AlignLeft);

    labels[0]->setToolTip(_STYLE_TOOLTIP_("Select input channel to be used "
                                          "in math operation"));
    inChSelector->setToolTip(_STYLE_TOOLTIP_("Select input channel to be used "
                                             "in math operation"));
    labels[1]->setToolTip(_STYLE_TOOLTIP_("Select math channel that the "
                                          "operation should be a component of"));
    mathChSelector->setToolTip(_STYLE_TOOLTIP_("Select math channel that the "
                                               "operation should be a component of"));
    labels[2]->setToolTip(_STYLE_TOOLTIP_("Select arithmetic operation to be "
                                          "performed on the input channel "));
    mathSelector->setToolTip(_STYLE_TOOLTIP_("Select arithmetic operation to be "
                                             "performed on the input channel "));
    deleteButton->setToolTip(_STYLE_TOOLTIP_("Delete math component"));

    for (int i = 0; i < 6; i++)
        mathChSelector->addItem(QString::number(i+1));

    QObject::connect(deleteButton, &QPushButton::pressed,
                     this, &UIMathChannelComponent::deleteComponent);
}

/**
 * @brief Update the list of math channels with the newly provided one
 * @param mathCh Array of new math channel IDs
 * @param size Size of mathCh array
 */
void UIMathChannelComponent::UpdateMathCh(int *mathCh, int size)
{
    int oldCh = mathChSelector->currentText().toInt(),
        index = -1;
    mathChSelector->clear();

    for (int i = 0; i < size; i++)
    {
        mathChSelector->addItem(QString::number(mathCh[i]));
        //  Attempt to preserve selected channel, if it
        //  exists in the new list save its index and
        //  activate it afterwards
        if (mathCh[i] == oldCh)
            index = i;
    }
    mathChSelector->setCurrentIndex(index);
}

/**
 * @brief Set selected input channel
 * @param inCh Input channel to take the data from for this math component
 */
void UIMathChannelComponent::SetInCh(int inCh)
{
    inChSelector->setValue(inCh);
}

/**
 * @brief Get currently selected input channel
 * @return Currently selected input channel
 */
int UIMathChannelComponent::GetInCh()
{
    return inChSelector->value();
}

/**
 * @brief Set selected math channel for this component
 * @param mathCh Math channel to be associated with this math component
 */
void UIMathChannelComponent::SetMathCh(int mathCh)
{
    bool valid = false;
    int i;

    //  Find if desired math channel exists
    for (i = 0; i < mathChSelector->count(); i++)
        if (mathChSelector->itemText(i).toInt() == mathCh)
        {
            valid = true;
            break;
        }

    //  If it exists, select it
    if (valid)
        mathChSelector->setCurrentIndex(i);
}

/**
 * @brief Get selected math channel for this component
 * @return
 */
int UIMathChannelComponent::GetMathCh()
{
    return mathChSelector->currentText().toInt();
}

/**
 * @brief Set math operation for this component
 * @param math id of math component
 */
void UIMathChannelComponent::SetMath(int math)
{
    mathSelector->setCurrentIndex(math);
}

/**
 * @brief Get selected math operation for this component
 * @return id of the selected math component
 */
int UIMathChannelComponent::GetMath()
{
    return mathSelector->currentIndex();
}

/**
 * @brief [Slot] Called by parent to delete this component
 */
void UIMathChannelComponent::deleteComponent()
{
    emit deleteRequested(_id);
}

/**
 * @brief Return layout of this component (used in deletion)
 * @return
 */
QHBoxLayout* UIMathChannelComponent::GetLayout()
{
    return layout;
}

/**
 * @brief Set this component's ID
 * @param id id to set
 */
void UIMathChannelComponent::SetID(uint8_t id)
{
    _id = id;
}

/**
 * @brief Get this component's ID
 * @return
 */
uint8_t  UIMathChannelComponent::GetID()
{
    return _id;
}
