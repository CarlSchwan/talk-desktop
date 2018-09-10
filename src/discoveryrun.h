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
    explicit DiscoveryRun(QString url, QObject *parent = nullptr);
    enum DiscoveryResult {
        Available = 0,
        OtherError = 1,
        HostNotFound = 2,
        Redirect = 3,
    };

public slots:
    void checkAvailability();
    void availabilityCheckFinished(QNetworkReply *reply);

signals:
    void protocolUnsupported(QString originalHost);
    void nextcloudDiscoveryFinished(int result, QUrl host, QString originalHost);

private:
    QString originalUrl;
    QUrl nc_server;
    QNetworkAccessManager nam;
};

#endif // DISCOVERYRUN_H
