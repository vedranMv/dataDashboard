#ifndef GRAPHHEADERWIDGET_H
#define GRAPHHEADERWIDGET_H

#include <stdint.h>

#include <QHBoxLayout>
#include <QComboBox>
#include <vector>

class graphHeaderWidget : public QObject
{
    Q_OBJECT

public:
    graphHeaderWidget(uint8_t chnnelNum, bool hasBoundaries);
    ~graphHeaderWidget();

    void UpdateChannelDropdown();

public slots:

    QHBoxLayout* GetLayout()
    {
        return _controlLayout;
    }

signals:
    void UpdateInputChannels(uint8_t *inChannels);

private slots:
    void ComboBoxUpdated(const int &);
private:


    QHBoxLayout *_controlLayout;
    uint8_t _n;
    bool _bounded;
    std::vector<QComboBox*>inCh;
};

#endif // GRAPHHEADERWIDGET_H
