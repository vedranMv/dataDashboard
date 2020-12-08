#ifndef MATHCHANNELCOMPONENT_H
#define MATHCHANNELCOMPONENT_H

#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QString>
#include <QHBoxLayout>
#include <QSpacerItem>

//#include "mainwindow.h"
#define _STYLE_TOOLTIP_(X) "<html><head/><body><p>" X "</p></body></html>"


class UIMathChannelComponent :  public QObject
{
    Q_OBJECT

public:
    UIMathChannelComponent(uint8_t id);
    ~UIMathChannelComponent();

    void UpdateMathCh(int *mathCh, int size);

    //  Input channel
    void SetInCh(int inCh);
    int GetInCh();
    //  Math channel
    void SetMathCh(int mathCh);
    int GetMathCh();
    //  Math operation
    void SetMath(int math);
    int GetMath();
    //  ID of the component
    void SetID(uint8_t id);
    uint8_t  GetID();

    QHBoxLayout* GetLayout();

    QLabel *labels[3];
    QComboBox *mathChSelector;
    QComboBox *mathSelector;
    QSpinBox *inChSelector;
    QPushButton *deleteButton;
    QHBoxLayout *layout;
    QSpacerItem *horSpacer;

signals:
    void deleteRequested(uint8_t id);
private slots:
    void deleteComponent();
private:
    //  Unique identifier
    int _id;
};

#endif // MATHCHANNELCOMPONENT_H
