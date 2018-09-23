#ifndef ROOM_H
#define ROOM_H

#include <QString>
#include "nextcloudaccount.h"

class Room
{
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString token READ token WRITE setToken)
    Q_PROPERTY(RoomType type READ type WRITE setType)
    Q_PROPERTY(int unread READ unreadMessages WRITE setUnreadMessages)
public:
    enum RoomType {
        Unknown = -1,
        OneOnOne = 1,
        Group = 2,
        Public = 3,
    };
    Room();

private:
    const NextcloudAccount *m_account = nullptr;
    QString m_name;
    QString m_token;
    RoomType m_type;
    int m_unread = 0;
    bool m_unread_mention;
    bool m_hasPassword;
    bool m_isFavorite;

public slots:
    const NextcloudAccount &account() const;
    QString name() const;
    QString token() const;
    RoomType type() const;
    int unreadMessages() const;
    bool unreadMention() const;
    bool hasPassword() const;
    bool isFavorite() const;

    Room& setName(const QString name);
    Room& setToken(const QString token);
    Room& setType(const RoomType type);
    Room& setAccount(const NextcloudAccount *account);
    Room& setUnreadMessages(const int unread);
    Room& setUnreadMention(const bool unreadMention);
    Room& setHasPassword(const bool hasPassword);
    Room& setFavorite(const bool isFavorite);
};

#endif // ROOM_H
