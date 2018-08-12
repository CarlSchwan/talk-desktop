#include "ncauthnamf.h"

QNetworkAccessManager *NcAuthNAMF::create(QObject *parent)
{
    qDebug() << "my QNAM!! :) ";
    QNetworkAccessManager *nam = new NcAuthNetworkAccessManager(parent);
    return nam;
}
