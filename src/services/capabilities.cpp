// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "capabilities.h"
#include "requestfactory.h"
#include "../nextcloudaccount.h"

#include <QMetaMethod>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Capabilities::Capabilities(NextcloudAccount *account)
    : QObject(account)
    , m_account(account)
{
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
    if (!m_available) {
        qDebug() << "capabilities have not been requested yet!";
        return color;
    }

    color.setNamedColor(m_capabilities
                        .find("theming").value().toObject()
                        .find("color").value().toString());

    if (!color.isValid()) {
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
    QUrl endpoint = QUrl(m_account->host());
    endpoint.setQuery("format=json");
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/cloud/capabilities");

    m_account->get(endpoint, [this](QNetworkReply *reply) -> void {
        const QByteArray payload = reply->readAll();
        const QJsonDocument apiResult = QJsonDocument::fromJson(payload);
        const QJsonObject q = apiResult.object();
        const QJsonObject root = q.find("ocs").value().toObject();
        const QJsonObject data = root.find("data").value().toObject();
        m_capabilities = data.find("capabilities").value().toObject();
        m_available = true;
    });
}

void Capabilities::checkTalkCapHash(QNetworkReply *reply) {
    if(reply->property("AccountID") != this->m_account->id()) {
        return;
    }
    const QByteArray newHash = reply->rawHeader("X-Nextcloud-Talk-Hash");
    if (m_talkCapHash == newHash) {
        return;
    }

    if (m_talkCapHash.isEmpty()) {
        m_talkCapHash = newHash;
        return;
    }
    m_available = false;
    m_talkCapHash = newHash;
    this->request();
}
