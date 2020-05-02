#ifndef REQUESTFACTORY_H
#define REQUESTFACTORY_H

#include <QNetworkRequest>
#include "accounts.h"

class RequestFactory
{
public:
    static QNetworkRequest getRequest(QUrl url, NextcloudAccount account);
    static QNetworkRequest getRequest(QUrl url, QString login, QString token);
};

#endif // REQUESTFACTORY_H
