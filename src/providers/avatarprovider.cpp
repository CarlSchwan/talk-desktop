#include "avatarprovider.h"

#include <QNetworkRequest>
#include "../services/accounts.h"

AvatarProvider::AvatarProvider() : AbstractNextcloudImageProvider()
{

}

QNetworkRequest AvatarProvider::getRequest(QString subject, NextcloudAccount account, const QSize &requestedSize)
{
    QUrl endpoint = QUrl(account.host());
    endpoint.setPath(endpoint.path() + "/avatar/" + subject + "/" + QString::number(requestedSize.height()));

    QNetworkRequest request(endpoint);

    QString concatanated = account.loginName() + ":" + account.password();
    QByteArray data = concatanated.toLocal8Bit().toBase64();
    QString authValue = "Basic " + data;

    request.setRawHeader("Authorization", authValue.toLocal8Bit());
    request.setRawHeader("OCS-APIRequest", "true");

    return request;
}
