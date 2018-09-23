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
    return QVariant();
}

QHash<int, QByteArray> RoomService::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
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
