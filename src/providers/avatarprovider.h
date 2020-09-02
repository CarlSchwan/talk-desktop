#ifndef AVATARPROVIDER_H
#define AVATARPROVIDER_H

#include "abstractnextcloudimageprovider.h"

class AvatarProvider : public AbstractNextcloudImageProvider
{
public:
    AvatarProvider();
    QNetworkRequest getRequest(QString subject, NextcloudAccount* account, const QSize &requestedSize);
};

#endif // AVATARPROVIDER_H
