#ifndef GRAPHHEADERWIDGET_H
#define GRAPHHEADERWIDGET_H

#include <stdint.h>

#include <QHBoxLayout>
#include <QComboBox>
#include <QVector>
#include <QLabel>

class graphHeaderWidget : public QObject
{
    Q_OBJECT

public:
    graphHeaderWidget(uint8_t chnnelNum);
    ~graphHeaderWidget();

    void AppendHorSpacer();
    void UpdateChannelDropdown();

    QVector<QLabel*>& GetLabels();
    QVector<QString> GetChLabels();
    QVector<uint8_t> GetSelectedChannels();

    void SetSelectedChannels(QVector<uint8_t> &selectedCh);

public slots:

    QHBoxLayout* GetLayout();

signals:
    void UpdateInputChannels(uint8_t *inChannels);

private slots:
    void ComboBoxUpdated(const int &);

private:

    QHBoxLayout *_mainLayout;

    //  List of dynamically constructed UI elements
    QVector<QComboBox*>_inCh;
    QVector<QLabel*>_inChLabel;
    QWidget *_parent;
    uint8_t *_inputChannelList;
};

#endif // GRAPHHEADERWIDGET_H
