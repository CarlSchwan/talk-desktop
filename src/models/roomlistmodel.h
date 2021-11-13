// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../room.h"
#include "accountmodel.h"
#include <QAbstractListModel>
#include <optional>
#include <vector>

class MessageEventModel;
class QNetworkAccessManager;
class QNetworkReply;
class NextcloudAccount;
class ParticipantModel;

class RoomListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)

    /// Model of message of the current room
    Q_PROPERTY(MessageEventModel *messageModel READ messageModel CONSTANT)

    /// This property holds the name of the currently selected room.
    Q_PROPERTY(QString currentName READ currentName NOTIFY roomChanged)

    /// This property holds the description of the currently selected room.
    Q_PROPERTY(QString currentDescription READ currentDescription NOTIFY roomChanged)

    /// This property holds whether the current room is a favorite.
    Q_PROPERTY(bool currentIsFavorite READ currentIsFavorite WRITE setCurrentIsFavorite NOTIFY roomChanged)

    /// This property holds the url of the room avatar.
    Q_PROPERTY(QString currentAvatarUrl READ currentAvatarUrl NOTIFY roomChanged)

    /// This property holds the description of the room.
    Q_PROPERTY(QString currentDescription READ currentDescription NOTIFY roomChanged)

    Q_PROPERTY(ParticipantModel *participants READ participants CONSTANT)

    Q_PROPERTY(bool hasOpenRoom READ hasOpenRoom NOTIFY hasOpenRoomChanged)
public:
    enum RoomRoles {
        NameRole = Qt::UserRole + 1,
        TokenRole,
        AccountRole,
        UnreadRole,
        MentionedRole,
        UserIdRole,
        ColorRole,
        LastMessageTextRole,
        LastMessageAuthorRole,
        LastMessageTimestampRole,
        LastMessageIsSystemMessageRole,
        TypeRole,
        ConversationNameRole,
    };

    explicit RoomListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    MessageEventModel *messageModel() const;
    bool isLoaded() const;
    QString currentName() const;
    QString currentAvatarUrl() const;
    QString currentDescription() const;
    bool currentIsFavorite() const;
    void setCurrentIsFavorite(bool isFavorite);
    ParticipantModel *participants() const;
    bool hasOpenRoom() const;

public slots:
    void loadRooms();
    void roomsLoadedFromAccount(QNetworkReply *reply, NextcloudAccount *account);
    void select(int index);
    void loadRoomFromAccount(NextcloudAccount *account);

signals:
    void newMessage(const QString &message);
    void roomChanged();
    void isLoadedChanged();
    void hasOpenRoomChanged();

private slots:
    std::vector<Room>::const_iterator findRoomByTokenAndAccount(const QString &token, NextcloudAccount *account) const;
    void onAccountsChanged();
    void onAccountUpdated();

private:
    AccountModel *m_accountModel = AccountModel::getInstance();
    std::vector<Room> m_rooms;
    QVector<QNetworkReply *> m_rooms_requests;
    QNetworkAccessManager *m_nam = nullptr;
    int m_pendingRequests = 0;
    QString activeToken;
    int activeAccountId;
    bool m_isLoaded = false;
    int m_lookIntoFuture = 0;
    MessageEventModel *m_messageModel = nullptr;
    std::optional<Room> m_currentRoom;
    ParticipantModel *m_participants;
    bool m_hasOpenRoom = false;
};
