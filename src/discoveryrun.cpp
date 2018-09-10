#include "discoveryrun.h"
#include <QNetworkRequest>

DiscoveryRun::DiscoveryRun(QString url, QObject *parent) : QObject(parent)
{
    originalUrl = url;
}

void DiscoveryRun::checkAvailability()
{
    nc_server = QUrl(originalUrl);
    // when only a domain was entered without port, the authority is empty and detection would not work
    if(nc_server.authority() == "" && nc_server.scheme() == "") {
        nc_server = QUrl("https://" + originalUrl);
    }

    // protocol checks, only http(s) supported, if nothing given we only go for https. It is 2018.
    if(nc_server.scheme() == "") {
        nc_server.setScheme("https");
    } else if (nc_server.scheme() != "http" && nc_server.scheme() != "https") {
        emit protocolUnsupported(originalUrl);
        // TODO: connect to in QML and test. Alternatively emit nextcloudDiscoveryFinished instead,
        // probably good enough and UI would still have material to react upon
        return;
    }

    // TODO: catch urls ending with index.php (not urgent)
    QString sep = "/";
    if(nc_server.path().endsWith(sep))
    {
        sep = "";
    }
    nc_server.setPath(nc_server.path() + sep + "status.php");
    qDebug() << "Looking for Nc at " << nc_server.url();
    QNetworkRequest request(nc_server);
    connect(&nam, &QNetworkAccessManager::finished, this, &DiscoveryRun::availabilityCheckFinished);
    nam.get(request);
}

void DiscoveryRun::availabilityCheckFinished(QNetworkReply *reply)
{
    qDebug() << "check finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    int result;
    switch (reply->error()) {
        case QNetworkReply::NoError:
            if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                result = DiscoveryResult::Available;
            } else {
                result = DiscoveryResult::Redirect;
            }
            break;
        case QNetworkReply::HostNotFoundError:
            result = DiscoveryResult::HostNotFound;
            break;
        default:
            result = DiscoveryResult::OtherError;
            break;
    }
    emit nextcloudDiscoveryFinished(result, reply->url(), originalUrl);

}
