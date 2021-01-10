#ifndef CHANNEL_H
#define CHANNEL_H

#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QString>

/**
 * @brief The Channel class
 *      Used to dynamically construct input channels and assign them a label
 */
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
    int channelId;
    QLineEdit *channelName;
};

#endif // CHANNEL_H
