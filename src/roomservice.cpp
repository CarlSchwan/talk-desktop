#include "roomservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <qdebug.h>

RoomService::RoomService(QObject *parent)
    : QAbstractListModel(parent)
{
    m_accounts = readAccounts();
}

int RoomService::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
   return m_rooms.length();
}

QVariant RoomService::data(const QModelIndex &index, int role) const
{
    if (role == NameRole)
    {
        return QVariant(m_rooms[index.row()].name());
    }

    if (role == TokenRole)
    {
        return QVariant(m_rooms[index.row()].token());
    }

    if (role == AccountRole)
    {
        return QVariant(m_rooms[index.row()].account().id());
    }

    return QVariant();
}

QHash<int, QByteArray> RoomService::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TokenRole] = "token";
    roles[AccountRole] = "accountId";
    return roles;
}

void RoomService::loadRooms() {
    m_pendingRequests = m_accounts.length();
    beginResetModel();
    if(m_pendingRequests > 0) {
        connect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
    }
    foreach (const NextcloudAccount account, m_accounts) {
        QUrl endpoint = QUrl(account.host());
        endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/room");
        endpoint.setQuery("format=json");
        QNetworkRequest request(endpoint);

        QString concatanated = account.loginName() + ":" + account.password();
        QByteArray data = concatanated.toLocal8Bit().toBase64();
        QString authValue = "Basic " + data;
        request.setRawHeader("Authorization", authValue.toLocal8Bit());
        request.setRawHeader("OCS-APIRequest", "true");

        m_nam.get(request);
    }
}

void RoomService::roomsLoadedFromAccount(QNetworkReply *reply) {
    m_pendingRequests--;

    qDebug() << "rooms loading finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed";
        return;
    }

    const NextcloudAccount* currentAccount = nullptr;
    int accounts = m_accounts.length();
    for(int i = 0; i < accounts; i++) {
        const NextcloudAccount* account = &m_accounts.at(i);
        if(account->host().authority() == reply->url().authority()) {
            currentAccount = account;
            qDebug() << "related account" << account->host().url();
            break;
        }
    }
    if(currentAccount == nullptr) {
        qDebug() << "Coult not figure out account for this result set";
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
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        return;
    }

    QJsonArray data = root.find("data").value().toArray();
    m_rooms.clear();
    foreach(const QJsonValue& value, data) {
        QJsonObject room = value.toObject();
        qDebug() << "read room" << room.value("name").toString();
        Room model;
        model
                .setAccount(currentAccount)
                .setName(room.value("name").toString())
                .setFavorite(room.value("isFavorite").toBool())
                .setHasPassword(room.value("hasPassword").toBool())
                .setToken(room.value("token").toString())
                .setType((Room::RoomType)room.value("type").toInt())
                .setUnreadMention(room.value("unreadMention").toBool())
                .setUnreadMessages(room.value("unreadMessages").toInt());
        m_rooms.append(model);
    }

    if(m_pendingRequests == 0) {
        endResetModel();
        disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
    }
}

Room RoomService::getRoom(QString token, int accountId) {
    QVector<Room>::iterator i;
    for(i = m_rooms.begin(); i != m_rooms.end(); i++) {
        if(i->token() == token && i->account().id() == accountId) {
            return *i;
        }
    }
    QException e;
    e.raise();
}

void RoomService::startPolling(QString token, int accountId) {
    activeToken = token;
    activeAccountId = accountId;
    lastKnownMessageId = 0;
    isPolling = true;
    pollRoom();
}

void RoomService::stopPolling() {
    isPolling = false;
}

NextcloudAccount RoomService::getAccountById(const int id) {
    QVector<NextcloudAccount>::iterator i;
    for(i = m_accounts.begin(); i != m_accounts.end(); i++) {
        if(i->id() == id) {
            return *i;
        }
    }
}

void RoomService::pollRoom() {
    if(!isPolling) {
        return;
    }
    connect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomPolled);
    NextcloudAccount account = getAccountById(activeAccountId);
    QUrl endpoint = QUrl(account.host());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/chat/" + activeToken);
    endpoint.setQuery("format=json&lookIntoFuture=1&timeout=30&lastKnownMessageId=" + QString::number(lastKnownMessageId));
    qDebug() << "Last known message id " << lastKnownMessageId;

    QNetworkRequest request(endpoint);

    QString concatanated = account.loginName() + ":" + account.password();
    QByteArray data = concatanated.toLocal8Bit().toBase64();
    QString authValue = "Basic " + data;

    request.setRawHeader("Authorization", authValue.toLocal8Bit());
    request.setRawHeader("OCS-APIRequest", "true");

    m_nam.get(request);
}

void RoomService::roomPolled(QNetworkReply *reply) {
    qDebug() << "polling for new messages finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomPolled);

    if(!isPolling) {
        return;
    }

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 304) {
        qDebug() << "no new messages";
        pollRoom();
        return;
    }

    if(reply->error() == QNetworkReply::ContentNotFoundError) {
        qDebug() << "Some server error?! check logs!";
        return;
    }

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed";
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
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        return;
    }

    QJsonArray data = root.find("data").value().toArray();
    foreach(const QJsonValue& value, data) {
        QJsonObject messageData = value.toObject();
        qDebug() << "read msg" << messageData.value("message").toString();
        lastKnownMessageId = messageData.value("id").toInt();
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
        QString message = messageData.value("message").toString();
        QJsonObject actorData = messageData.value("messageParameters").toObject().value("actor").toObject();

        message.replace("{actor}", actorData.value("name").toString());
        // TODO: try to send the whole json object and do the magic on QML side
        // TODO: try with mentions
        emit newMessage(message);
    }

    pollRoom();
}
