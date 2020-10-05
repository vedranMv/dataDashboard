#ifndef MATHCHANNELCOMPONENT_H
#define MATHCHANNELCOMPONENT_H

#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QString>

class MathChannelComponent
{
public:
    MathChannelComponent();

    void UpdateMathCh(int *mathCh, int size);
    void SetInCh(int inCh);
    void SetMathCh(int mathCh);
    void SetName(QString name);
    void SetMath(int math);

    QLabel *labels[4];
    QComboBox *mathChSelector;
    QComboBox *mathSelector;
    QSpinBox *inChSelector;
    QLineEdit *chName;
};

#endif // MATHCHANNELCOMPONENT_H
