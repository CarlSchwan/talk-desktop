#include "capabilities.h"
#include "requestfactory.h"

#include <QMetaMethod>
#include <QJsonDocument>
#include <QJsonObject>

Capabilities::Capabilities(NextcloudAccount *account) {
    m_account = account;
}

bool Capabilities::areAvailable() const {
    return m_available;
}

void Capabilities::request() {
    if(m_reply && m_reply->isRunning()) {
        return;
    }
    connect(&m_nam, SIGNAL(finished(QNetworkReply*)), SLOT(requestFinished(QNetworkReply*)));

    QUrl endpoint = QUrl(m_account->host());
    endpoint.setQuery("format=json");
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/cloud/capabilities");

    QNetworkRequest request = RequestFactory::getRequest(endpoint, m_account);
    m_reply = m_nam.get(request);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(handleError(QNetworkReply::NetworkError)));
}

void Capabilities::requestFinished(QNetworkReply *reply) {
    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed, code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(m_nam.networkAccessible() != QNetworkAccessManager::Accessible) {
            qDebug() << "Network not accessible";
        }
        disconnect(&m_nam, &QNetworkAccessManager::finished, this, &Capabilities::requestFinished);
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    QJsonObject data = root.find("data").value().toObject();
    m_capabilities = data.find("capabilities").value().toObject();
    m_available = true;

    disconnect(&m_nam, &QNetworkAccessManager::finished, this, &Capabilities::requestFinished);
}
