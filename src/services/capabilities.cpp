#include "capabilities.h"
#include "requestfactory.h"

#include <QMetaMethod>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Capabilities::Capabilities(NextcloudAccount *account) {
    m_account = account;
}

bool Capabilities::areAvailable() const {
    return m_available;
}

int Capabilities::getConversationApiLevel() const
{
    if (m_capabilities
            .find("spreed").value().toObject()
            .find("features").value().toArray()
            .toVariantList().contains("conversation-v4"))
    {
        return 4;
    }

    if (m_capabilities
            .find("spreed").value().toObject()
            .find("features").value().toArray()
            .toVariantList().contains("conversation-v2"))
    {
        return 2;
    }

    return 1;
}

QColor Capabilities::primaryColor() const {
    QColor color;
    if(!m_available) {
        qDebug() << "capabilities have not been requested yet!";
        return color;
    }

    color.setNamedColor(m_capabilities
                        .find("theming").value().toObject()
                        .find("color").value().toString());

    if (!color.isValid())
    {
        // fallback to Nextcloud-blue when no color is provided
        color.setNamedColor("#0082c9");
    }

    return color;
}

QUrl Capabilities::logoUrl() const {
    if(!m_available) {
        qDebug() << "capabilities have not been requested yet!";
        return QUrl();
    }

    return QUrl::fromUserInput(m_capabilities
                               .find("theming").value().toObject()
                               .find("logo").value().toString()
                               );
}

QString Capabilities::name() const {
    if(!m_available) {
        qDebug() << "capabilities have not been requested yet!";
        return "";
    }

    return m_capabilities
            .find("theming").value().toObject()
            .find("name").value().toString();
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
    disconnect(&m_nam, &QNetworkAccessManager::finished, this, &Capabilities::requestFinished);

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
}

void Capabilities::checkTalkCapHash(QNetworkReply *reply) {
    if(reply->property("AccountID") != this->m_account->id()) {
        return;
    }
    QByteArray newHash = reply->rawHeader("X-Nextcloud-Talk-Hash");
    if(m_talkCapHash == newHash) {
        return;
    } else if (m_talkCapHash == "") {
        m_talkCapHash = newHash;
        return;
    }
    m_available = false;
    m_talkCapHash = newHash;
    this->request();
}
