#ifndef DISCOVERYRUN_H
#define DISCOVERYRUN_H

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class DiscoveryRun : public QObject
{
    Q_OBJECT
public:
    explicit DiscoveryRun(const QString &url, QObject *parent = nullptr);
    explicit DiscoveryRun(const QString &url, const QString &loginName, const QString &token, QObject *parent = nullptr);
    enum DiscoveryResult {
        Available = 0,
        OtherError = 1,
        HostNotFound = 2,
        Redirect = 3,
    };

public slots:
    void checkAvailability();
    void verifyCredentials();
    void availabilityCheckFinished(QNetworkReply *reply);
    void credentialsCheckFinished(QNetworkReply *reply);

signals:
    void protocolUnsupported(QString originalHost);
    void nextcloudDiscoveryFinished(int result, QUrl host, QString originalHost);
    void verifyCredentialsFinished(bool isVerified, QString host, QString originalUrl, QString loginName, QString token, QString userId);

private slots:
    void testCredentials(int result, QUrl host, QString originalHost);

private:
    QString originalUrl;
    QUrl nc_server;
    QString m_loginName;
    QString m_token;
    QNetworkAccessManager nam;
};

#endif // DISCOVERYRUN_H
