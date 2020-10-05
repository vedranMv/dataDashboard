#include "mathchannelcomponent.h"

MathChannelComponent::MathChannelComponent()
{
    labels[0] = new QLabel("Input channel");
    labels[1] = new QLabel("Math channel");
    labels[2] = new QLabel("Math");
    labels[3] = new QLabel("Channel label");

    mathChSelector = new QComboBox();
    inChSelector = new QSpinBox();
    chName = new QLineEdit();

    mathSelector = new QComboBox();
    mathSelector->addItem("Add");
    mathSelector->addItem("Subtract");
}

void MathChannelComponent::UpdateMathCh(int *mathCh, int size)
{
    int oldCh = mathChSelector->currentText().toInt(),
        index = -1;
    mathChSelector->clear();

    for (int i = 0; i < size; i++)
    {
        mathChSelector->addItem(QString::number(mathCh[i]));
        //  Attempt to preserve selected channel, if it
        //  exists in the new list save it's index and
        //  activate it afterwrds
        if (mathCh[i] == oldCh)
            index = i;
    }
    mathChSelector->setCurrentIndex(index);
}
void MathChannelComponent::SetInCh(int inCh)
{
    inChSelector->setValue(inCh);
}
void MathChannelComponent::SetMathCh(int mathCh)
{
    bool valid = false;
    int i;

    for (i = 0; i < mathChSelector->count(); i++)
        if (mathChSelector->itemText(i).toInt() == mathCh)
        {
            valid = true;
            break;
        }

    if (valid)
        mathChSelector->setCurrentIndex(i);
}
void MathChannelComponent::SetName(QString name)
{
    chName->setText(name);
}

void MathChannelComponent::SetMath(int math)
{
    //  0 - Add
    //  1 - Subtract
    mathSelector->setCurrentIndex(math);
}

