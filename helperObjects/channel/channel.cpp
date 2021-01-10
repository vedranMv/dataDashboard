#include "channel.h"

Channel::Channel(QString label, int id, QString name)
{
    chLabel = new QLabel(label);
    channelName = new QLineEdit();

    channelId = id;
    channelName->setText(name);
}

/**
 * @brief Update channel components
 * @param label new label for the channel
 * @param id new channel id
 * @param name new channel name
 */
void Channel::Update(QString label, int id, QString name)
{
    chLabel->setText(label);
    channelId = id;
    channelName->setText(name);
}

QString Channel::GetLabel()
{
    return chLabel->text();
}

int  Channel::GetId()
{
    return channelId;
}

QString Channel::GetName()
{
    return channelName->text();
}

Channel::~Channel()
{
    delete chLabel;
    delete channelName;
}
