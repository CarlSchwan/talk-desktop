#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include "discoveryrun.h"

class Discovery : public QObject
{
    Q_OBJECT

public:
    explicit Discovery(QObject *parent = nullptr);

public slots:
    void discoverInstance(QString url);
    void discoveryFinished(int result, QUrl host, QString originalUrl);
    void verifyCredentials(QString host, QString loginName, QString token);
    void verifyCredentialsFinished(bool isVerified, QString host, QString originalUrl, QString loginName, QString token);

signals:
    void protocolUnsupported(QString originalHost);
    void discoverySuccessful(QString host, QString originalHost);
    void discoveryFailed(QString message, QString host, QString originalHost);
    void credentialsChecked(bool isVerified);

private:
    QNetworkAccessManager m_nam;
};

#endif // DISCOVERY_H
