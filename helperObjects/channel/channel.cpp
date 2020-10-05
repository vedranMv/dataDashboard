#include "channel.h"

Channel::Channel(QString label, int id, QString name)
{
    chLabel = new QLabel(label);
    channelId = new QSpinBox();
    channelName = new QLineEdit();

    channelId->setValue(id);
    channelName->setText(name);
}


void Channel::Update(QString label, int id, QString name)
{
    chLabel->setText(label);
    channelId->setValue(id);
    channelName->setText(name);
}

QString Channel::GetLabel()
{
    return chLabel->text();
}

int  Channel::GetId()
{
    return channelId->value();
}

QString Channel::GetName()
{
    return channelName->text();
}

Channel::~Channel()
{
    delete chLabel;
    delete channelId;
    delete channelName;
}
