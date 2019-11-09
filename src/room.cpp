#include "room.h"

Room::Room()
{

}

QString Room::name() const
{
    return m_name;
}

Room& Room::setName(const QString name)
{
    m_name = name;
    return *this;
}

QString Room::token() const
{
    return m_token;
}

Room& Room::setToken(const QString token)
{
    m_token = token;
    return *this;
}

Room::RoomType Room::type() const
{
    return m_type;
}

Room& Room::setType(const RoomType type)
{
    m_type = type;
    return *this;
}

const NextcloudAccount& Room::account() const
{
    return *m_account;
}

Room& Room::setAccount(const NextcloudAccount *account)
{
    m_account = account;
    return *this;
}

int Room::unreadMessages() const
{
    return m_unread;
}

Room& Room::setUnreadMessages(const int unread)
{
    m_unread = unread;
    return *this;
}

bool Room::unreadMention() const
{
    return m_unread_mention;
}

Room& Room::setUnreadMention(const bool unreadMention)
{
    m_unread_mention = unreadMention;
    return *this;
}

bool Room::hasPassword() const
{
    return m_hasPassword;
}

Room& Room::setHasPassword(const bool hasPassword)
{
    m_hasPassword = hasPassword;
    return *this;
}

bool Room::isFavorite() const
{
    return m_isFavorite;
}

Room& Room::setFavorite(const bool isFavorite)
{
    m_isFavorite = isFavorite;
    return *this;
}

int Room::lastActivity() const {
    return m_lastActivity;
}

Room& Room::setLastActivity(const int lastActivity)
{
    m_lastActivity = lastActivity;
    return *this;
}

bool Room::operator ==(const Room &toCompare) const {
    return toCompare.account().id() == account().id()
            && toCompare.token() == token();
}
