#ifndef NCAUTHNAMF_H
#define NCAUTHNAMF_H

#include "ncauthnetworkaccessmanager.h"
#include <QQmlNetworkAccessManagerFactory>

class NcAuthNAMF : public QQmlNetworkAccessManagerFactory
{
public:
    virtual QNetworkAccessManager *create(QObject *parent);
};

#endif // NCAUTHNAMF_H
