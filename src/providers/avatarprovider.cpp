#include "avatarprovider.h"

#include <QNetworkRequest>
#include "../services/accounts.h"
#include "../services/requestfactory.h"

AvatarProvider::AvatarProvider() : AbstractNextcloudImageProvider()
{
}

QNetworkRequest AvatarProvider::getRequest(QString subject, NextcloudAccount* account, const QSize &requestedSize)
{
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/avatar/" + subject + "/" + QString::number(requestedSize.height()));

    return RequestFactory::getRequest(endpoint, account);
}
