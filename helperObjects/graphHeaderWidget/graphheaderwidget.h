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
    graphHeaderWidget(uint8_t chnnelNum, QString ParentWinName="");
    ~graphHeaderWidget();

    void AppendHorSpacer();
    void UpdateChannelDropdown();

    QVector<QLabel*>& GetLabels();
    QVector<QString> GetChLabels();
    QVector<uint8_t> GetSelectedChannels();

    void SetSelectedChannels(QVector<uint8_t> &selectedCh);
    void SetChToolTip(uint8_t id, QString toolTip);

public slots:

    QHBoxLayout* GetLayout();

signals:
    void UpdateInputChannels(uint8_t *inChannels);
    void logLine(const QString &line);

private slots:
    void ComboBoxUpdated(const int &);

private:

    QHBoxLayout *_mainLayout;
    QString _parentWinName;

    //  List of dynamically constructed UI elements
    QVector<QComboBox*>_inCh;
    QVector<QLabel*>_inChLabel;
    QWidget *_parent;
    uint8_t *_inputChannelList;
};

#endif // GRAPHHEADERWIDGET_H
