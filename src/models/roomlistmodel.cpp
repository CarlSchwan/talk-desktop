// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roomlistmodel.h"
#include "../constants/ConversationTypeClass.h"
#include "../services/capabilities.h"
#include "messageeventmodel.h"
#include "participantmodel.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <iterator>
#include <qdebug.h>

// https://cloud.nextcloud.com/ocs/v2.php/core/autocomplete/get?search=supp&itemType=call&itemId=new&shareTypes[]=0&shareTypes[]=1&shareTypes[]=7

RoomListModel::RoomListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_messageModel(new MessageEventModel(this))
    , m_participants(new ParticipantModel(this))
{
    connect(m_accountModel, &QAbstractItemModel::columnsInserted, this, [this](const QModelIndex &parent, int first, int last) {
        Q_UNUSED(parent)
        Q_UNUSED(first)
        Q_UNUSED(last)
        onAccountsChanged();
    });
    connect(m_accountModel, &QAbstractItemModel::rowsRemoved, this, &RoomListModel::onAccountsChanged);
    connect(m_accountModel, &QAbstractItemModel::dataChanged, this, &RoomListModel::onAccountUpdated);
}

int RoomListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rooms.size();
}

const NextcloudAccount *Room::account() const
{
    return m_account;
}

QVariant RoomListModel::data(const QModelIndex &index, int role) const
{
    auto room = m_rooms[index.row()];
    switch (role) {
    case RoomRole:
        return QVariant::fromValue(room);
    case NameRole:
        return room->name();
    case TokenRole:
        return room->token();
    case UserIdRole:
        return room->account()->userId();
    case UnreadRole:
        return room->unreadMessages();
    case MentionedRole:
        return room->unreadMention();
    case ColorRole:
        if (room->account()->colorOverride().isValid()) {
            return room->account()->colorOverride();
        }
        return room->account()->capabilities()->primaryColor();
    case AccountRole:
        return room->account()->id();
    case LastMessageTextRole:
        return room->lastMessageText();
    case LastMessageAuthorRole:
        return room->lastMessageAuthor();
    case LastMessageTimestampRole:
        return room->lastMessageTimestamp();
    case LastMessageIsSystemMessageRole:
        return room->lastMessageIsSystemMessage();
    case TypeRole:
        return room->type();
    case ConversationNameRole:
        return room->name();
    }
    return {};
}

QHash<int, QByteArray> RoomListModel::roleNames() const
{
    return {
        {RoomRole, QByteArrayLiteral("room")},
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
        {ConversationNameRole, QByteArrayLiteral("conversationName")},
    };
}

void RoomListModel::loadRooms()
{
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

void RoomListModel::loadRoomFromAccount(NextcloudAccount *account)
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

void RoomListModel::roomsLoadedFromAccount(QNetworkReply *reply, NextcloudAccount *account)
{
    const auto apiResult = QJsonDocument::fromJson(reply->readAll()).object()[QLatin1String("ocs")].toObject();
    qDebug() << apiResult;

    int statusCode = apiResult[QLatin1String("meta")].toObject()[QLatin1String("statuscode")].toInt();
    if (statusCode != 200) {
        qDebug() << "unexpected OCS code " << statusCode;
        if (statusCode == 0) {
            qDebug() << "Error:" << QJsonDocument(apiResult).toJson(QJsonDocument::Indented) << reply->url();
        }
        return;
    }

    const QJsonArray data = apiResult[QLatin1String("data")].toArray();
    for (const QJsonValue &value : data) {
        QJsonObject roomObject = value.toObject();
        const auto token = roomObject[QLatin1String("token")].toString();
        bool updated = false;
        for (const auto room : qAsConst(m_rooms)) {
            if (room->token() == token) {
                room->updateFromJsonObject(roomObject);
                updated = true;
                break;
            }
        }

        if (!updated) {
            beginInsertRows(QModelIndex(), m_rooms.size(), m_rooms.size());
            m_rooms.push_back(new Room(account, roomObject));
            endInsertRows();
        }
    }

    if (!m_isLoaded) {
        m_isLoaded = true;
        Q_EMIT isLoadedChanged();
    }
}

void RoomListModel::onAccountsChanged()
{
    qDebug() << "RoomListModel acting on Accounts change";
    beginResetModel();
    m_rooms.clear();
    endResetModel();
    loadRooms();
}

void RoomListModel::onAccountUpdated()
{
    qDebug() << "RoomListModel acting on Account update";
    beginResetModel();
    endResetModel();
}

MessageEventModel *RoomListModel::messageModel() const
{
    return m_messageModel;
}

bool RoomListModel::isLoaded() const
{
    return m_isLoaded;
}

QString RoomListModel::currentName() const
{
    return m_currentRoom ? m_currentRoom->name() : QString();
}

QString RoomListModel::currentDescription() const
{
    qDebug() << "desc1" << (m_currentRoom ? m_currentRoom->description() : "empty");
    return m_currentRoom ? m_currentRoom->description() : QString();
}

QString RoomListModel::currentAvatarUrl() const
{
    if (m_currentRoom->type() == ConversationType::OneToOne) {
        return QStringLiteral("image://avatar/") + QString::number(m_currentRoom->account()->id()) + QLatin1Char('/') + m_currentRoom->name() + QLatin1Char('/');
    }
    return QString(); // no avatar
}

bool RoomListModel::currentIsFavorite() const
{
    return m_currentRoom ? m_currentRoom->isFavorite() : false;
}

void RoomListModel::setCurrentIsFavorite(bool isFavorite)
{
    // TODO
}

ParticipantModel *RoomListModel::participants() const
{
    return m_participants;
}

bool RoomListModel::hasOpenRoom() const
{
    return m_hasOpenRoom;
}

void RoomListModel::select(int index)
{
    if (m_hasOpenRoom == false) {
        m_hasOpenRoom = true;
        Q_EMIT hasOpenRoomChanged();
    }
    auto room = m_rooms[index];
    m_currentRoom = room;
    auto account = AccountModel::getInstance()->getAccountById(room->account()->id());
    m_messageModel->setRoom(room->token(), room->lastReadMessage(), account);
    m_participants->setRoom(room->token(), account);
}
