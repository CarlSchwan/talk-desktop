// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QJsonObject>
#include "nextcloudaccount.h"

/// A talk conversation
class Room
{
public:
    enum RoomType {
        Unknown = -1,
        OneOnOne = 1,
        Group = 2,
        Public = 3,
    };

    Room(NextcloudAccount *account, const QJsonObject &obj);

    bool operator ==(const Room &toCompare) const;

    const NextcloudAccount &account() const;
    QString name() const;
    QString description() const;
    QString token() const;
    int lastReadMessage() const;
    RoomType type() const;
    int unreadMessages() const;
    bool unreadMention() const;
    bool hasPassword() const;
    bool isFavorite() const;
    int lastActivity() const;
    QString lastMessageText() const;
    QString lastMessageAuthor() const;
    uint lastMessageTimestamp() const;
    bool lastMessageIsSystemMessage() const;
    QString conversationName() const;

    Room *setLastReadMessage(int lastReadMessage);
    Room *setName(const QString &name);
    Room *setDescription(const QString &description);
    Room *setToken(const QString &token);
    Room *setType(const RoomType type);
    Room *setAccount(const NextcloudAccount *account);
    Room *setUnreadMessages(const int unread);
    Room *setUnreadMention(const bool unreadMention);
    Room *setHasPassword(const bool hasPassword);
    Room *setFavorite(const bool isFavorite);
    Room *setLastActivity(const int lastActivity);
    Room *setLastMessage(const QString &lastMessage, const QString &lastAuthor, const uint timestamp, const bool isSystemMessage);
    Room *setConversationName(const QString &name);

private:
    const NextcloudAccount *m_account = nullptr;
    QString m_name;
    QString m_token;
    QString m_description;
    RoomType m_type;
    int m_unread = 0;
    bool m_unread_mention;
    bool m_hasPassword;
    bool m_isFavorite;
    int m_lastActivity;
    QString m_lastMessageText;
    QString m_lastMessageAuthor;
    int m_lastReadMessage;
    uint m_lastMessageTimestamp;
    bool m_lastMessageIsSystemMessage;
    QString m_conversationName;
};
