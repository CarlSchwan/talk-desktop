#include "discoveryrun.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

DiscoveryRun::DiscoveryRun(QString url, QObject *parent) : QObject(parent)
{
    originalUrl = url;
}

DiscoveryRun::DiscoveryRun(QString url, QString loginName, QString token, QObject *parent) : QObject(parent)
{
    originalUrl = url;
    m_loginName = loginName;
    m_token = token;
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
    disconnect(&nam, &QNetworkAccessManager::finished, this, &DiscoveryRun::availabilityCheckFinished);
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

    QString requestUrl(reply->url().toString());
    requestUrl.chop(QString("status.php").length());
    emit nextcloudDiscoveryFinished(result, QUrl(requestUrl), originalUrl);
}

void DiscoveryRun::verifyCredentials()
{
    connect(this, &DiscoveryRun::nextcloudDiscoveryFinished,
            this, &DiscoveryRun::testCredentials);
    checkAvailability();
}

void DiscoveryRun::testCredentials(int result, QUrl host, QString originalHost)
{
    if(result != DiscoveryResult::Available) {
        emit verifyCredentialsFinished(false, host.toString(), originalUrl, m_loginName, m_token);
    }
    host.setPath(host.path() + "ocs/v2.php/cloud/user");
    host.setQuery("format=json");
    QNetworkRequest request(host);
    QString concatanated = m_loginName + ":" + m_token;
    QByteArray data = concatanated.toLocal8Bit().toBase64();
    QString authValue = "Basic " + data;
    request.setRawHeader("Authorization", authValue.toLocal8Bit());
    request.setRawHeader("OCS-APIRequest", "true");
    connect(&nam, &QNetworkAccessManager::finished, this, &DiscoveryRun::credentialsCheckFinished);
    nam.get(request);
}

void DiscoveryRun::credentialsCheckFinished(QNetworkReply *reply)
{
    disconnect(&nam, &QNetworkAccessManager::finished, this, &DiscoveryRun::credentialsCheckFinished);

    qDebug() << "check finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed";
        emit verifyCredentialsFinished(false, nc_server.toString(), originalUrl, m_loginName, m_token);
        return;
    }

    QJsonDocument apiResult = QJsonDocument::fromJson(reply->readAll());
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    QJsonObject meta = root.find("meta").value().toObject();
    QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        emit verifyCredentialsFinished(false, nc_server.toString(), originalUrl, m_loginName, m_token);
        return;
    }

    QString userId = root.find("data").value().toObject().find("id").value().toString();
    qDebug() << "found user ID: " << userId;
    emit verifyCredentialsFinished(true, nc_server.toString(), originalUrl, m_loginName, m_token);
}
