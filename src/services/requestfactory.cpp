#include "requestfactory.h"

#include <qdebug.h>

QNetworkRequest RequestFactory::getRequest(QUrl url, NextcloudAccount* account) {
    return RequestFactory::getRequest(url, account->loginName(), account->password());
}

QNetworkRequest RequestFactory::getRequest(QUrl url, QString login, QString token) {
    QNetworkRequest request(url);

    QString concatanated = login + ":" + token;
    QByteArray data = concatanated.toLocal8Bit().toBase64();
    QString authValue = "Basic " + data;

    request.setRawHeader("Authorization", authValue.toLocal8Bit());
    request.setRawHeader("User-Agent", "Mozilla/5.0 Nextcloud Talk for SailfishOS/1.0");

    if(url.path().indexOf("/ocs/") >= 0) {
        request.setRawHeader("OCS-APIRequest", "true");
    }

    return request;
}
