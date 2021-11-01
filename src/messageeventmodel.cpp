// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messageeventmodel.h"
#include <QDateTime>
#include <QHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include "nextcloudaccount.h"
#include "services/accounts.h"
#include <QMetaMethod>

MessageEventModel::MessageEventModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

MessageEventModel::~MessageEventModel()
{
}

void MessageEventModel::addMessages(const QJsonObject &obj)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    auto &message = m_messages.emplace_back();
    message.text = obj["message"].toString().replace("{actor}", obj["actorDisplayName"].toString())
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "gt;")
        .replace("\n", "<br />");

    const QJsonObject parameters = obj.value("messageParameters").toObject();

    for (const QString &placeholder: parameters.keys()) {
        const auto subString = "{" + placeholder + "}";
        if (placeholder == "user") {
            message.text = message.text.replace(message.text.indexOf(subString), subString.size(), parameters[placeholder].toObject()["name"].toString());
        } else if (placeholder.startsWith("mention-")) {
            message.text = message.text.replace(message.text.indexOf(subString), subString.size(), "<a href=''>" + parameters[placeholder].toObject()["name"].toString() + "</a>");
        }

        //QString name = parameters.value(placeholder).toObject().value("name").toString();
        //message.text.replace("{" + placeholder + "}", name, Qt::CaseSensitive);
    }

    message.obj = obj;

    if (message.text == "{file}") {
        message.type = SingleLinkImageMessage;
        message.text = "File TODO";
    }
    endInsertRows();
}

bool MessageEventModel::hasFileAttachment(const QJsonObject &messageParameters) const
{
    for (const QString &key : messageParameters.keys()) {
        const auto &value = messageParameters[key].toObject();
        if (value.contains(QLatin1String("type")) && value["type"].toString() == QLatin1String("file")) {
            return true;
        };
    }
    return false;
}

bool MessageEventModel::hasGeoLocation(const QJsonObject &messageParameters) const
{
    for (const QString &key : messageParameters.keys()) {
        const auto &value = messageParameters[key].toObject();
        if (value.contains(QLatin1String("type")) && value["type"].toString() == QLatin1String("geo-location")) {
            return true;
        };
    }
    return false;
}

QUrl MessageEventModel::getImageUrl(const QJsonObject &messageParameters) const
{
    for (const QString &key : messageParameters.keys()) {
        const auto &value = messageParameters[key].toObject();
        if (value.contains(QLatin1String("type")) && value["type"].toString() == QLatin1String("file")) {
            //getUrlForFilePreviewWithFileId();
            /*if(!isVoiceMessage()){
                return (ApiUtils.getUrlForFilePreviewWithFileId(getActiveUser().getBaseUrl(),
                                                                individualHashMap.get("id"), NextcloudTalkApplication.Companion.getSharedApplication().getResources().getDimensionPixelSize(R.dimen.maximum_file_preview_size)));
            }*/
        }
    }
    return QUrl();
}

void MessageEventModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

int MessageEventModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_messages.size();
}

QVariant MessageEventModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &message = m_messages[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            return message.text;
        case ActorDisplayNameRole:
            return message.obj["actorId"].toString();
        case DateRole:
            return QDateTime::fromMSecsSinceEpoch(message.obj["timestamp"].toInt() * 1000);
        case AvatarRole:
            return QLatin1String("image://avatar/") + QString::number(m_account->id()) + QLatin1Char('/') + message.obj["actorId"].toString() + QLatin1Char('/');
        case ShowAuthorRole: {
            if (index.row() == 0) {
                return true;
            }
            const auto &lastMessage = m_messages[index.row() - 1];
            return lastMessage.obj["actorId"].toString() != message.obj["actorId"].toString();
        }
        case IsHiglightedRole:
            return false; // TODO
        case IsLocalUserRole:
            return false; // TODO
        case EventTypeRole:
            return message.type;
        case ImagePathRole:
            if (message.type != SingleLinkImageMessage) {
                return {};
            }
            const auto fileId = message.obj[QLatin1String("messageParameters")].toObject()[QLatin1String("file")].toObject()[QLatin1String("id")].toString();
            return QLatin1String("image://preview/") + QString::number(m_account->id()) + QLatin1Char('/') + fileId + QLatin1Char('/');
    }

    return {};
}

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {ActorDisplayNameRole, QByteArrayLiteral("actorDisplayName")},
        {DateRole, QByteArrayLiteral("time")},
        {AvatarRole, QByteArrayLiteral("avatar")},
        {ShowAuthorRole, QByteArrayLiteral("showAuthor")},
        {IsHiglightedRole, QByteArrayLiteral("isHighlighted")},
        {IsLocalUserRole, QByteArrayLiteral("isLocalUser")}
    };
}

void MessageEventModel::setRoom(const QString &token, int lastReadMessage, NextcloudAccount *account)
{
    if (m_token == token && m_account == account) {
        return;
    }
    m_account = account;
    m_token = token;
    m_lastReadMessage = lastReadMessage;
    m_lookIntoFuture = 0;
    clear();

    // Start polling loop
    connect(this, &MessageEventModel::pollingDone,
            this, &MessageEventModel::pollRoom,
            Qt::QueuedConnection);
    pollRoom();
}

void MessageEventModel::pollRoom()
{
    if (!m_account || m_token.isEmpty()) {
        return;
    }

    const int lastKnownMessageId = m_db.lastKnownMessageId(m_account, m_token);
    QString includeLastKnown = m_lookIntoFuture == 0 ? QLatin1String("1") : QLatin1String("0");
    QUrl endpoint = QUrl(m_account->host());
    endpoint.setPath(endpoint.path() + QLatin1String("/ocs/v2.php/apps/spreed/api/v1/chat/") + m_token);

    QUrlQuery urlQuery;
    urlQuery.setQueryItems({
        {QStringLiteral("format"), QStringLiteral("json")},
        {QStringLiteral("lookIntoFuture"), QString::number(m_lookIntoFuture)},
        {QStringLiteral("includeLastKnown"), includeLastKnown},
        {QStringLiteral("lastKnownMessageId"), QString::number(m_lastReadMessage)},
    });
    endpoint.setQuery(urlQuery);
    
    qDebug() << endpoint;

    const auto token = m_token;

    m_account->get(endpoint, [this, token](QNetworkReply *reply) {
        roomPolled(reply, token);
    });
}

void MessageEventModel::roomPolled(QNetworkReply *reply, const QString &token) {
    qDebug() << "polling for new messages finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_account == nullptr || m_token.isEmpty()) {
        return;
    }

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 304) {
        qDebug() << "no new messages";
        Q_EMIT pollingDone();
        return;
    }

    if(reply->error() == QNetworkReply::ContentNotFoundError) {
        qDebug() << "Some server error?! check logs! Polling stopped.";
        return;
    }

    if(reply->error() == QNetworkReply::TimeoutError) {
        qDebug() << "timeout…";
        Q_EMIT pollingDone();
        return;
    }

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "issue occured:" << reply->errorString() << "→ Polling stopped!";
        return;
    }

    const QByteArray payload = reply->readAll();
    const QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    const QJsonObject q = apiResult.object();
    const QJsonObject root = q.find("ocs").value().toObject();
    qDebug() << "JSON" << apiResult;
    const QJsonObject meta = root.find("meta").value().toObject();
    const QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt() << "→ polling stopped";
        return;
    }

    const QJsonArray data = root.find("data").value().toArray();
    int start, end, step;
    if(m_lookIntoFuture == 0) {
        start = data.size() - 1;
        end = -1;
        step = -1;
        // this happens just once to get some history
        m_lookIntoFuture = 1;
    } else {
        start = 0;
        end = data.size();
        step = 1;
    }

    for(auto i = start; i != end; i += step) {
        const QJsonValue value = data.at(i);
        const QJsonObject messageData = value.toObject();
        const int msgId = messageData.value("id").toInt();
        if(msgId > m_lastReadMessage) {
             m_lastReadMessage = msgId;
        }

        const QString systemMessage = messageData.value("systemMessage").toString();
        if(systemMessage == "call_left"
           || systemMessage == "call_started"
           || systemMessage == "conversation_created"
        ) {
            // some system message we simply ignore:
            // - created because maybe it is not so important
            // - calls because they are not supported
            continue;
        }

        addMessages(value.toObject());
    }

    Q_EMIT pollingDone();
}

void MessageEventModel::sendMessage(const QString &messageText, int replyToId) {
    QUrl endpoint = QUrl(m_account->host());
    endpoint.setPath(endpoint.path() + QLatin1String("/ocs/v2.php/apps/spreed/api/v1/chat/") + m_token);

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("message"), QUrl::toPercentEncoding(messageText));
    if(replyToId > -1) {
        urlQuery.addQueryItem(QLatin1String("replyTo"), QString::number(replyToId));
    }
    m_account->post(endpoint, urlQuery, nullptr);
}

void MessageEventModel::emitAfterActiveRoomChanged(const QString &token, NextcloudAccount *account) {
    //QDBusConnection bus = QDBusConnection::sessionBus();
    //QDBusMessage event = QDBusMessage::createSignal("/conversation", "org.nextcloud.talk", "afterActiveConversationChanged");
    //event << token << account;
    //bus.send(event);
}

