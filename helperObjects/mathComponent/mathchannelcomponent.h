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


class MathChannelComponent :  public QObject
{
    Q_OBJECT

public:
    MathChannelComponent(int id);
    ~MathChannelComponent();

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
    void SetID(int id);
    int  GetID();

    QHBoxLayout* GetLayout();

    QLabel *labels[3];
    QComboBox *mathChSelector;
    QComboBox *mathSelector;
    QSpinBox *inChSelector;
    QPushButton *deleteButton;
    QHBoxLayout *layout;
    QSpacerItem *horSpacer;

signals:
    void deleteRequested(int id);
private slots:
    void deleteComponent();
private:
    //  Unique identifier
    int _id;
};

#endif // MATHCHANNELCOMPONENT_H
