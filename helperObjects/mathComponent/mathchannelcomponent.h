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

/**
 * @brief The UIMathChannelComponent class
 *      Object dynamically constructed in UI to represent match channel
 *      components under 'Math channel' page.
 */
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


    QComboBox *mathChSelector;
    QComboBox *mathSelector;
    QSpinBox *inChSelector;
    QPushButton *deleteButton;
    QHBoxLayout *layout;

signals:
    void deleteRequested(uint8_t id);
private slots:
    void deleteComponent();
private:
    //  Unique identifier
    int _id;
};

#endif // MATHCHANNELCOMPONENT_H
