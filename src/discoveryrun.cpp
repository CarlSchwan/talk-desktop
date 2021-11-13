#include "discoveryrun.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

DiscoveryRun::DiscoveryRun(const QString &url, QObject *parent)
    : QObject(parent)
    , originalUrl(url)
{
}

DiscoveryRun::DiscoveryRun(const QString &url, const QString &loginName, const QString &token, QObject *parent)
    : QObject(parent)
    , originalUrl(url)
    , m_loginName(loginName)
    , m_token(token)
{
}

void DiscoveryRun::checkAvailability()
{
    nc_server = QUrl(originalUrl);

    // when only a domain was entered without port, the authority is empty and detection would not work
    if (nc_server.authority().isEmpty() && nc_server.scheme().isEmpty()) {
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
    QString sep = QStringLiteral("/");
    if (nc_server.path().endsWith(sep)) {
        sep = QString();
    }
    QUrl requestUrl(nc_server.toString());
    requestUrl.setPath(requestUrl.path() + sep + "status.php");
    qDebug() << "Looking for Nc at " << requestUrl.url();
    QNetworkRequest request(requestUrl);
    auto reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        availabilityCheckFinished(reply);
    });
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

    Q_EMIT nextcloudDiscoveryFinished(result, nc_server, originalUrl);
}

void DiscoveryRun::verifyCredentials()
{
    connect(this, &DiscoveryRun::nextcloudDiscoveryFinished,
            this, &DiscoveryRun::testCredentials);
    checkAvailability();
}

void DiscoveryRun::testCredentials(int result, QUrl host, QString originalHost)
{
    Q_UNUSED(originalHost);
    if(result != DiscoveryResult::Available) {
        emit verifyCredentialsFinished(false, host.toString(), originalUrl, m_loginName, m_token, "");
    }
    // TODO we can just use nc_server instead of host and originalHost
    host.setPath(host.path() + "/ocs/v2.php/cloud/user");
    host.setQuery("format=json");
    QNetworkRequest request(host);

    QString concatanated = m_loginName + ":" + m_token;
    QByteArray data = concatanated.toLocal8Bit().toBase64();
    QString authValue = "Basic " + data;

    request.setRawHeader("Authorization", authValue.toLocal8Bit());
#ifdef KDE_EDITION
    request.setRawHeader("User-Agent", "Mozilla/5.0 Nextcloud Talk Desktop/Qt");
#else
    request.setRawHeader("User-Agent", "Mozilla/5.0 Nextcloud Talk for SailfishOS/1.0");
#endif
    request.setRawHeader("OCS-APIRequest", "true");

    auto reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        credentialsCheckFinished(reply);
    });
}

void DiscoveryRun::credentialsCheckFinished(QNetworkReply *reply)
{
    qDebug() << "check finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed";
        emit verifyCredentialsFinished(false, nc_server.toString(), originalUrl, m_loginName, m_token, "");
        return;
    }

    const QJsonDocument apiResult = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject q = apiResult.object();
    const QJsonObject root = q.find("ocs").value().toObject();
    const QJsonObject meta = root.find("meta").value().toObject();
    const QJsonValue statuscode = meta.find("statuscode").value();
    if (statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        emit verifyCredentialsFinished(false, nc_server.toString(), originalUrl, m_loginName, m_token, "");
        return;
    }

    const QString userId = root.find("data").value().toObject().find("id").value().toString();
    qDebug() << "found user ID: " << userId;
    emit verifyCredentialsFinished(true, nc_server.toString(), originalUrl, m_loginName, m_token, userId);
}
