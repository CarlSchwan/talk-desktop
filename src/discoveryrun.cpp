#include "discoveryrun.h"
#include <QNetworkRequest>

DiscoveryRun::DiscoveryRun(QString url, QObject *parent) : QObject(parent)
{
    originalUrl = url;
}

void DiscoveryRun::checkAvailability()
{
    nc_server = QUrl(originalUrl);
    if(nc_server.scheme() == "") {
        nc_server.setScheme("https");
    } else if (nc_server.scheme() != "http" && nc_server.scheme() != "https") {
        emit protocolUnsupported(originalUrl);
        // TODO: connect to in QML and test
        return;
    }

    QString sep = "/";
    if(nc_server.path().endsWith(sep))
    {
        sep = "";
    }
    nc_server.setPath(sep + "status.php");

    QNetworkRequest request(nc_server);
    connect(&nam, &QNetworkAccessManager::finished, this, &DiscoveryRun::availabilityCheckFinished);
    nam.get(request);
}

void DiscoveryRun::availabilityCheckFinished(QNetworkReply *reply)
{
    qDebug() << "check finished " << reply->error();
    switch (reply->error()) {
        case QNetworkReply::NoError:
            emit nextcloudDiscoveryFinished(DiscoveryResult::Available, reply->url(), originalUrl);
            break;
        case QNetworkReply::HostNotFoundError:
            emit nextcloudDiscoveryFinished(DiscoveryResult::HostNotFound, reply->url(), originalUrl);
            break;
        default:
            emit nextcloudDiscoveryFinished(DiscoveryResult::OtherError, reply->url(), originalUrl);
            break;
    }

}
