#ifndef CHANNEL_H
#define CHANNEL_H

#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QString>

class Channel
{
public:
    Channel(QString label, int id, QString name);
    void Update(QString label, int id, QString name);

    QString GetLabel();
    int     GetId();
    QString GetName();

    ~Channel();

    QLabel *chLabel;
    QSpinBox *channelId;
    QLineEdit *channelName;
};

#endif // CHANNEL_H
