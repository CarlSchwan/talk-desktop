#include "previewprovider.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include "../services/accounts.h"
#include "../services/requestfactory.h"

PreviewProvider::PreviewProvider()
    : AbstractNextcloudImageProvider()
{
}

QNetworkRequest PreviewProvider::getRequest(QString subject, NextcloudAccount* account, const QSize &requestedSize)
{
    const QUrlQuery urlQuery({
        {QStringLiteral("fileId"), subject},
        {QStringLiteral("x"), QString::number(requestedSize.width())},
        {QStringLiteral("y"), QString::number(requestedSize.height())}
    });

    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/core/preview");
    endpoint.setQuery(urlQuery);

    return account->setupRequest(endpoint);
}
