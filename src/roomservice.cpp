// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roomservice.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMetaMethod>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <qdebug.h>
#include "services/requestfactory.h"
#include "services/capabilities.h"
#include <iterator>
#include "messageeventmodel.h"
#include "services/participants.h"


RoomService::RoomService(QObject *parent)
    : QAbstractListModel(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_messageModel(new MessageEventModel(this))
    , m_participants(new Participants(this))
{
    connect(m_accountService, &QAbstractItemModel::modelReset, this, &RoomService::onAccountsChanged);
    connect(m_accountService, &QAbstractItemModel::rowsRemoved, this, &RoomService::onAccountsChanged);
    connect(m_accountService, &QAbstractItemModel::dataChanged, this, &RoomService::onAccountUpdated);
}

int RoomService::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rooms.size();
}

QVariant RoomService::data(const QModelIndex &index, int role) const
{
    const Room &room = m_rooms[index.row()];
    switch (role) {
        case NameRole:
            return room.name();
        case TokenRole:
            return room.token();
        case AccountRole:
            return room.account().id();
        case UserIdRole:
            return room.account().userId();
        case UnreadRole:
            return room.unreadMessages();
        case MentionedRole:
            return room.unreadMention();
        case ColorRole:
            if (room.account().colorOverride().isValid()) {
                return room.account().colorOverride();
            }
            return room.account().capabilities()->primaryColor();
        case LastMessageTextRole:
            return room.lastMessageText();
        case LastMessageAuthorRole:
            return room.lastMessageAuthor();
        case LastMessageTimestampRole:
            return room.lastMessageTimestamp();
        case LastMessageIsSystemMessageRole:
            return room.lastMessageIsSystemMessage();
        case TypeRole:
            return room.type();
        case ConversationNameRole:
            return room.conversationName();
    }
    return {};
}

QHash<int, QByteArray> RoomService::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TokenRole] = "token";
    roles[AccountRole] = "accountId";
    roles[UserIdRole] = "accountUserId";
    roles[UnreadRole] = "unreadMessages";
    roles[MentionedRole] = "unreadMention";
    roles[ColorRole] = "primaryColor";
    roles[LastMessageTextRole] = "lastMessageText";
    roles[LastMessageAuthorRole] = "lastMessageAuthor";
    roles[LastMessageTimestampRole] = "lastMessageTimestamp";
    roles[LastMessageIsSystemMessageRole] = "lastMessageIsSystemMessage";
    roles[TypeRole] = "conversationType";
    roles[ConversationNameRole] = "conversationName";
    return roles;
}

void RoomService::loadRooms() {
    const auto accounts = m_accountService->getAccounts();
    for (NextcloudAccount* account : accounts) {
        if (account->password().isEmpty()) {
            continue;
        }

        if (!account->capabilities()->areAvailable()) {
            account->capabilities()->request();
            continue;
        }

        QUrl endpoint = QUrl(account->host());
        const QString apiV = "v" + QString::number(account->capabilities()->getConversationApiLevel());
        endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/" + apiV + "/room");
        endpoint.setQuery("format=json");
        QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
        auto reply = m_nam->get(request);
        QObject::connect(reply, &QNetworkReply::finished, [this, reply, account]() {
            if (200 != reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)) {
                qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->url();
                return;
            }

            account->capabilities()->checkTalkCapHash(reply);
            roomsLoadedFromAccount(reply, account);
        });
    }
}

void RoomService::roomsLoadedFromAccount(QNetworkReply *reply, NextcloudAccount *account) {
    auto apiResult = QJsonDocument::fromJson(reply->readAll()).object()[QLatin1Literal("ocs")].toObject();
    qDebug() << apiResult;

    int statusCode = apiResult[QLatin1Literal("meta")].toObject()[QLatin1Literal("statuscode")].toInt();
    if(statusCode != 200) {
        qDebug() << "unexpected OCS code " << statusCode;
        if(statusCode == 0) {
            qDebug() << "Error:" << QJsonDocument(apiResult).toJson(QJsonDocument::Indented) << reply->url();
        }
        return;
    }

    const QJsonArray data = apiResult["data"].toArray();
    for (const QJsonValue& value : data) {
        QJsonObject room = value.toObject();
        auto position = findRoomByTokenAndAccount(room["token"].toString(), account->id());
        if (position != m_rooms.cend()) {
            m_rooms.erase(position);
            m_rooms.emplace(position, account, room);
            int pos = std::distance(m_rooms.cbegin(), position);
            dataChanged(index(pos, 0), index(pos, 0), {});
        } else {
            beginInsertRows(QModelIndex(), m_rooms.size(), m_rooms.size());
            m_rooms.emplace_back(account, room);
            endInsertRows();
        }
    }
}

void RoomService::startPolling(const QString &token, int accountId) {
    activeToken = token;
    activeAccountId = accountId;
    m_messageModel->setAccountId(accountId);
    m_isPolling = true;
    m_lookIntoFuture = 0;
    pollRoom();
    emitAfterActiveRoomChanged(token, accountId);
    m_messageModel->clear();
    m_currentRoom = *findRoomByTokenAndAccount(token, accountId);
    m_participants->setTokenAndAccountId(token, accountId);
    Q_EMIT isLoadedChanged();
}

void RoomService::emitAfterActiveRoomChanged(const QString &token, int accountId) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage event = QDBusMessage::createSignal("/conversation", "org.nextcloud.talk", "afterActiveConversationChanged");
    event << token << accountId;
    bus.send(event);
}

bool RoomService::isPolling(const QString &token, int accountId) {
    return activeToken == token
            && activeAccountId == accountId
            && m_isPolling;
}

void RoomService::stopPolling() {
    m_isPolling = false;
}

std::vector<Room>::const_iterator RoomService::findRoomByTokenAndAccount(const QString &token, const int accountId) const {
    auto it = m_rooms.cbegin();

    while (it != m_rooms.cend()) {
        const auto &room = *it;
        if (room.token() == token && room.account().id() == accountId) {
            return it;
        }
        it++;
    }
    return it;
}

void RoomService::pollRoom() {
    if(!m_isPolling) {
        return;
    }
    NextcloudAccount* account = m_accountService->getAccountById(activeAccountId);
    if (!account) {
        qDebug() << "Failed to poll for room" << activeAccountId;
        return;
    }
    int lastKnownMessageId = m_db.lastKnownMessageId(activeAccountId, activeToken);
    QString includeLastKnown = m_lookIntoFuture == 0 ? "1" : "0";
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/chat/" + activeToken);
    endpoint.setQuery("format=json&lookIntoFuture=" + QString::number(m_lookIntoFuture) +
                      "&timeout=30&lastKnownMessageId=" + QString::number(lastKnownMessageId) +
                      "&includeLastKnown=" + includeLastKnown);

    QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
    auto reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        roomPolled(reply);
    });
}

void RoomService::roomPolled(QNetworkReply *reply) {
    qDebug() << "polling for new messages finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(!m_isPolling) {
        return;
    }

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 304) {
        qDebug() << "no new messages";
        pollRoom();
        return;
    }

    if(reply->error() == QNetworkReply::ContentNotFoundError) {
        qDebug() << "Some server error?! check logs! Polling stopped.";
        return;
    }

    if(reply->error() == QNetworkReply::TimeoutError) {
        qDebug() << "timeout…";
        pollRoom();
        return;
    }

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "issue occured:" << reply->errorString() << "→ Polling stopped!";
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    //qDebug() << "JSON" << payload;
    QJsonObject meta = root.find("meta").value().toObject();
    QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt() << "→ polling stopped";
        return;
    }

    QJsonArray data = root.find("data").value().toArray();
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
        QJsonValue value = data.at(i);
        QJsonObject messageData = value.toObject();
        int msgId = messageData.value("id").toInt();
        if(msgId > m_db.lastKnownMessageId(activeAccountId, activeToken)) {
            // do not lower value when we fetched history
            m_db.setLastKnownMessageId(activeAccountId, activeToken, msgId);
        }

        QString systemMessage = messageData.value("systemMessage").toString();
        if(systemMessage == "call_left"
           || systemMessage == "call_started"
           || systemMessage == "conversation_created"
        ) {
            // some system message we simply ignore:
            // - created because maybe it is not so important
            // - calls because they are not supported
            continue;
        }

        m_messageModel->addMessages(value.toObject());
    }

    pollRoom();
}

void RoomService::sendMessage(const QString &messageText, int replyToId) {
    NextcloudAccount* account = m_accountService->getAccountById(activeAccountId);
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/chat/" + activeToken);

    QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray payload = QString("message=" + QUrl::toPercentEncoding(messageText)).toUtf8();
    if(replyToId > -1) {
        payload += QString("&replyTo=" + QString::number(replyToId)).toUtf8();
    }

    m_nam->post(request, payload);
}

void RoomService::onAccountsChanged() {
    qDebug() << "RoomService acting on Accounts change";
    beginResetModel();
    m_rooms.clear();
    endResetModel();
    loadRooms();
}

void RoomService::onAccountUpdated() {
    qDebug() << "RoomService acting on Account update";
    beginResetModel();
    endResetModel();
}

MessageEventModel *RoomService::messageModel() const
{
    return m_messageModel;
}

bool RoomService::isLoaded() const
{
    return m_isPolling;
}

QString RoomService::currentName() const
{
    return m_currentRoom ? m_currentRoom->name() : QString();
}

QString RoomService::currentAvatarUrl() const
{
    return QString(); // TODO
}

bool RoomService::currentIsFavorite() const
{
    return m_currentRoom ? m_currentRoom->isFavorite() : false;
}

void RoomService::setCurrentIsFavorite(bool isFavorite)
{
    // TODO
}

QString RoomService::currentDescription() const
{
    return QString(); // TODO
}

Participants *RoomService::participants() const
{
    return m_participants;
}
