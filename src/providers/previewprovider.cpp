#include "previewprovider.h"
#include <QNetworkRequest>
#include "../services/accounts.h"
#include "../services/requestfactory.h"

PreviewProvider::PreviewProvider() : AbstractNextcloudImageProvider()
{

}

QNetworkRequest PreviewProvider::getRequest(QString subject, NextcloudAccount account, const QSize &requestedSize)
{
    QUrl endpoint = QUrl(account.host());
    endpoint.setPath(endpoint.path() + "/core/preview");
    endpoint.setQuery("fileId=" + subject
                      + "&x=" + QString::number(requestedSize.width())
                      + "&y=" + QString::number(requestedSize.height())
                      );

    return RequestFactory::getRequest(endpoint, account);
}
