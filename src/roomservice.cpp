// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roomservice.h"
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
    connect(m_accountModel, &QAbstractItemModel::columnsInserted, this, [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent)
        Q_UNUSED(first)
        Q_UNUSED(last)
        onAccountsChanged();
    });
    connect(m_accountModel, &QAbstractItemModel::rowsRemoved, this, &RoomService::onAccountsChanged);
    connect(m_accountModel, &QAbstractItemModel::dataChanged, this, &RoomService::onAccountUpdated);
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
        case AccountRole:
            return room.account().id();
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
    return {
        {NameRole, QByteArrayLiteral("name")},
        {TokenRole, QByteArrayLiteral("token")},
        {AccountRole, QByteArrayLiteral("accountId")},
        {UserIdRole, QByteArrayLiteral("accountUserId")},
        {UnreadRole, QByteArrayLiteral("unreadMessages")},
        {MentionedRole, QByteArrayLiteral("unreadMention")},
        {ColorRole, QByteArrayLiteral("primaryColor")},
        {LastMessageTextRole, QByteArrayLiteral("lastMessageText")},
        {LastMessageAuthorRole, QByteArrayLiteral("lastMessageAuthor")},
        {LastMessageTimestampRole, QByteArrayLiteral("lastMessageTimestamp")},
        {LastMessageIsSystemMessageRole, QByteArrayLiteral("lastMessageIsSystemMessage")},
        {TypeRole, QByteArrayLiteral("conversationType")},
        {ConversationNameRole, QByteArrayLiteral("conversationName")}
    };
}

void RoomService::loadRooms() {
    qDebug() << "start loading rooms";
    const auto accounts = m_accountModel->getAccounts();
    for (NextcloudAccount *account : accounts) {
        if (account->password().isEmpty()) {
            continue;
        }

        if (!account->capabilities()->areAvailable()) {
            account->capabilities()->request([this, account]() {
                loadRoomFromAccount(account);
            });
            continue;
        }
        loadRoomFromAccount(account);
    }
}

void RoomService::loadRoomFromAccount(NextcloudAccount *account)
{
    QUrl endpoint = QUrl(account->host());
    const QString apiV = "v" + QString::number(account->capabilities()->getConversationApiLevel());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/" + apiV + "/room");
    endpoint.setQuery("format=json");

    account->get(endpoint, [this, account](QNetworkReply *reply) {
        // room endpoint carries talk specific indicator whether capabilities have changed
        account->capabilities()->checkTalkCapHash(reply, [this, account] {
            loadRoomFromAccount(account);
        });
        roomsLoadedFromAccount(reply, account);
    });
}

void RoomService::roomsLoadedFromAccount(QNetworkReply *reply, NextcloudAccount *account) {
    const auto apiResult = QJsonDocument::fromJson(reply->readAll()).object()[QLatin1Literal("ocs")].toObject();

    int statusCode = apiResult[QLatin1Literal("meta")].toObject()[QLatin1Literal("statuscode")].toInt();
    if(statusCode != 200) {
        qDebug() << "unexpected OCS code " << statusCode;
        if(statusCode == 0) {
            qDebug() << "Error:" << QJsonDocument(apiResult).toJson(QJsonDocument::Indented) << reply->url();
        }
        return;
    }

    const QJsonArray data = apiResult[QLatin1Literal("data")].toArray();
    for (const QJsonValue& value : data) {
        QJsonObject room = value.toObject();
        auto position = findRoomByTokenAndAccount(room[QLatin1Literal("token")].toString(), account);
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
    
    if (!m_isLoaded) {
        m_isLoaded = true;
        Q_EMIT isLoadedChanged();
    }
}

std::vector<Room>::const_iterator RoomService::findRoomByTokenAndAccount(const QString &token, NextcloudAccount *account) const {
    auto it = m_rooms.cbegin();

    while (it != m_rooms.cend()) {
        const auto &room = *it;
        if (room.token() == token && &room.account() == account) {
            return it;
        }
        it++;
    }
    return it;
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
    return m_isLoaded;
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

bool RoomService::hasOpenRoom() const
{
    return m_hasOpenRoom;
}

void RoomService::select(int index)
{
    if (m_hasOpenRoom == false) {
        m_hasOpenRoom = true;
        Q_EMIT hasOpenRoomChanged();
    }
    Room &room = m_rooms[index];
    auto account = AccountModel::getInstance()->getAccountById(room.account().id());
    m_messageModel->setRoom(room.token(), room.lastReadMessage(), account);
}
