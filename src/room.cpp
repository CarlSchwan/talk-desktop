// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "room.h"

Room::Room(NextcloudAccount *account, const QJsonObject &obj)
    : QObject(account)
    , m_account(account)
    , m_actorId(obj["actorId"].toString())
    , m_actorType(obj["actorType"].toString())
    , m_attendeeId(obj["attendeeId"].toInt())
    , m_attendeePermissions(obj["attendeePermissions"].toInt())
    , m_attendeePin(obj["attendeePin"].toString())
    , m_callFlag(obj["callFlag"].toInt())
    , m_callPermissions(obj["callPermissions"].toInt())
    , m_canDeleteConversation(obj["canDeleteConversation"].toBool())
    , m_canEnableSIP(obj["canEnableSIP"].toBool())
    , m_canLeaveConversation(obj["canLeaveConversation"].toBool())
    , m_canStartCall(obj["canStartCall"].toBool())
    , m_defaultPermissions(obj["defaultPermissions"].toInt())
    , m_description(obj["description"].toString())
    , m_displayName(obj["displayName"].toString())
    , m_hasCall(obj["hasCall"].toBool())
    , m_hasPassword(obj["hasPassword"].toBool())
    , m_id(obj["id"].toInt())
    , m_isFavorite(obj["isFavorite"].toBool())
    , m_lastActivity(obj["lastActivity"].toInt())
    , m_lastCommonReadMessage(obj["lastCommonReadMessage"].toInt())
    , m_lastPing(obj["lastPing"].toBool())
    , m_lastReadMessage(obj["lastReadMessage"].toInt())
    , m_listable(obj["listable"].toInt())
    , m_lobbyState(obj["lobbyState"].toInt())
    , m_lobbyTimer(obj["lobbyTimer"].toInt())
    , m_name(obj["name"].toString())
    , m_notificationCalls(obj["notificationCalls"].toInt())
    , m_notificationLevel(obj["notificationLevel"].toInt())
    , m_objectId(obj["objectId"].toString())
    , m_participantFlags(obj["participantFlags"].toInt())
    , m_participantType(obj["participantType"].toInt())
    , m_permissions(obj["permissions"].toInt())
    , m_readAll(obj["readAll"].toBool())
    , m_sessionId(obj["sessionId"].toString())
    , m_sipEnabled(obj["sipEnabled"].toBool())
    , m_token(obj["token"].toString())
    , m_type(static_cast<Room::Types>(obj["type"].toInt()))
    , m_unreadMention(obj["unreadMention"].toBool())
    , m_unreadMentionDirect(obj["unreadMentionDirect"].toBool())
    , m_unreadMessages(obj["unreadMessages"].toInt())
{
    if (obj.contains("lastMessage")) {
        const QJsonObject lastMessage = obj.value("lastMessage").toObject();
        QString message = lastMessage.value("message").toString();
        const QJsonObject parameters = lastMessage.value("messageParameters").toObject();

        for (const QString &placeholder: parameters.keys()) {
            const QString name = parameters.value(placeholder).toObject().value("name").toString();
            message.replace("{" + placeholder + "}", name, Qt::CaseSensitive);
        }

        m_lastMessageText = message;
        m_lastMessageAuthor = lastMessage.value("actorDisplayName").toString();
        m_lastMessageTimestamp = lastMessage.value("timestamp").toVariant().toUInt();
        m_lastMessageIsSystemMessage = lastMessage.value("systemMessage").toString() != "";
    }
}

void Room::updateFromJsonObject(const QJsonObject &obj)
{
    setActorId(obj["actorId"].toString());
    setActorType(obj["actorType"].toString());
    setAttendeeId(obj["attendeeId"].toInt());
    setAttendeePermissions(obj["attendeePermissions"].toInt());
    setAttendeePin(obj["attendeePin"].toString());
    setCallFlag(obj["callFlag"].toInt());
    setCallPermissions(obj["callPermissions"].toInt());
    setCanDeleteConversation(obj["canDeleteConversation"].toBool());
    setCanEnableSIP(obj["canEnableSIP"].toBool());
    setCanLeaveConversation(obj["canLeaveConversation"].toBool());
    setCanStartCall(obj["canStartCall"].toBool());
    setDefaultPermissions(obj["defaultPermissions"].toInt());
    setDescription(obj["description"].toString());
    setDisplayName(obj["displayName"].toString());
    setHasCall(obj["hasCall"].toBool());
    setHasPassword(obj["hasPassword"].toBool());
    setId(obj["id"].toInt());
    setIsFavorite(obj["isFavorite"].toBool());
    setLastActivity(obj["lastActivity"].toInt());
    setLastCommonReadMessage(obj["lastCommonReadMessage"].toInt());
    setLastPing(obj["lastPing"].toBool());
    setLastReadMessage(obj["lastReadMessage"].toInt());
    setListable(obj["listable"].toInt());
    setLobbyState(obj["lobbyState"].toInt());
    setLobbyTimer(obj["lobbyTimer"].toInt());
    setName(obj["name"].toString());
    setNotificationCalls(obj["notificationCalls"].toInt());
    setNotificationLevel(obj["notificationLevel"].toInt());
    setObjectId(obj["objectId"].toString());
    setParticipantFlags(obj["participantFlags"].toInt());
    setParticipantType(obj["participantType"].toInt());
    setPermissions(obj["permissions"].toInt());
    setReadAll(obj["readAll"].toBool());
    setSessionId(obj["sessionId"].toString());
    setSipEnabled(obj["sipEnabled"].toBool());
    setToken(obj["token"].toString());
    setType(static_cast<Room::Types>(obj["type"].toInt()));
    setUnreadMention(obj["unreadMention"].toBool());
    setUnreadMentionDirect(obj["unreadMentionDirect"].toBool());
    setUnreadMessages(obj["unreadMessages"].toInt());

    if (obj.contains("lastMessage")) {
        const QJsonObject lastMessage = obj.value("lastMessage").toObject();
        QString message = lastMessage.value("message").toString();
        const QJsonObject parameters = lastMessage.value("messageParameters").toObject();

        for (const QString &placeholder: parameters.keys()) {
            const QString name = parameters.value(placeholder).toObject().value("name").toString();
            message.replace("{" + placeholder + "}", name, Qt::CaseSensitive);
        }

        setLastMessageText(message);
        setLastMessageAuthor(lastMessage.value("actorDisplayName").toString());
        setLastMessageTimestamp(lastMessage.value("timestamp").toVariant().toUInt());
        setLastMessageIsSystemMessage(lastMessage.value("systemMessage").toString() != "");
    }
}

QString Room::actorId() const
{
    return m_actorId;
}

void Room::setActorId(const QString &actorId)
{
    if (m_actorId == actorId) {
        return;
    }
    m_actorId = actorId;
    Q_EMIT actorIdChanged();
}

QString Room::actorType() const
{
    return m_actorType;
}

void Room::setActorType(const QString &actorType)
{
    if (m_actorType == actorType) {
        return;
    }
    m_actorType = actorType;
    Q_EMIT actorTypeChanged();
}

int Room::attendeeId() const
{
    return m_attendeeId;
}

void Room::setAttendeeId(int attendeeId)
{
    if (m_attendeeId == attendeeId) {
        return;
    }
    m_attendeeId = attendeeId;
    Q_EMIT attendeeIdChanged();
}

int Room::attendeePermissions() const
{
    return m_attendeePermissions;
}

void Room::setAttendeePermissions(int attendeePermissions)
{
    if (m_attendeePermissions == attendeePermissions) {
        return;
    }
    m_attendeePermissions = attendeePermissions;
    Q_EMIT attendeePermissionsChanged();
}

QString Room::attendeePin() const
{
    return m_attendeePin;
}

void Room::setAttendeePin(const QString &attendeePin)
{
    if (m_attendeePin == attendeePin) {
        return;
    }
    m_attendeePin = attendeePin;
    Q_EMIT attendeePinChanged();
}

int Room::callFlag() const
{
    return m_callFlag;
}

void Room::setCallFlag(int callFlag)
{
    if (m_callFlag == callFlag) {
        return;
    }
    m_callFlag = callFlag;
    Q_EMIT callFlagChanged();
}

int Room::callPermissions() const
{
    return m_callPermissions;
}

void Room::setCallPermissions(int callPermissions)
{
    if (m_callPermissions == callPermissions) {
        return;
    }
    m_callPermissions = callPermissions;
    Q_EMIT callPermissionsChanged();
}

bool Room::canDeleteConversation() const
{
    return m_canDeleteConversation;
}

void Room::setCanDeleteConversation(bool canDeleteConversation)
{
    if (m_canDeleteConversation == canDeleteConversation) {
        return;
    }
    m_canDeleteConversation = canDeleteConversation;
    Q_EMIT canDeleteConversationChanged();
}

bool Room::canEnableSIP() const
{
    return m_canEnableSIP;
}

void Room::setCanEnableSIP(bool canEnableSIP)
{
    if (m_canEnableSIP == canEnableSIP) {
        return;
    }
    m_canEnableSIP = canEnableSIP;
    Q_EMIT canEnableSIPChanged();
}

bool Room::canLeaveConversation() const
{
    return m_canLeaveConversation;
}

void Room::setCanLeaveConversation(bool canLeaveConversation)
{
    if (m_canLeaveConversation == canLeaveConversation) {
        return;
    }
    m_canLeaveConversation = canLeaveConversation;
    Q_EMIT canLeaveConversationChanged();
}

bool Room::canStartCall() const
{
    return m_canStartCall;
}

void Room::setCanStartCall(bool canStartCall)
{
    if (m_canStartCall == canStartCall) {
        return;
    }
    m_canStartCall = canStartCall;
    Q_EMIT canStartCallChanged();
}

int Room::defaultPermissions() const
{
    return m_defaultPermissions;
}

void Room::setDefaultPermissions(int defaultPermissions)
{
    if (m_defaultPermissions == defaultPermissions) {
        return;
    }
    m_defaultPermissions = defaultPermissions;
    Q_EMIT defaultPermissionsChanged();
}

QString Room::description() const
{
    return m_description;
}

void Room::setDescription(const QString &description)
{
    if (m_description == description) {
        return;
    }
    m_description = description;
    Q_EMIT descriptionChanged();
}

QString Room::displayName() const
{
    return m_displayName;
}

void Room::setDisplayName(const QString &displayName)
{
    if (m_displayName == displayName) {
        return;
    }
    m_displayName = displayName;
    Q_EMIT displayNameChanged();
}

bool Room::hasCall() const
{
    return m_hasCall;
}

void Room::setHasCall(bool hasCall)
{
    if (m_hasCall == hasCall) {
        return;
    }
    m_hasCall = hasCall;
    Q_EMIT hasCallChanged();
}

bool Room::hasPassword() const
{
    return m_hasPassword;
}

void Room::setHasPassword(bool hasPassword)
{
    if (m_hasPassword == hasPassword) {
        return;
    }
    m_hasPassword = hasPassword;
    Q_EMIT hasPasswordChanged();
}

int Room::id() const
{
    return m_id;
}

void Room::setId(int id)
{
    if (m_id == id) {
        return;
    }
    m_id = id;
    Q_EMIT idChanged();
}

bool Room::isFavorite() const
{
    return m_isFavorite;
}

void Room::setIsFavorite(bool isFavorite)
{
    if (m_isFavorite == isFavorite) {
        return;
    }
    m_isFavorite = isFavorite;
    Q_EMIT isFavoriteChanged();
}

int Room::lastActivity() const
{
    return m_lastActivity;
}

void Room::setLastActivity(int lastActivity)
{
    if (m_lastActivity == lastActivity) {
        return;
    }
    m_lastActivity = lastActivity;
    Q_EMIT lastActivityChanged();
}

int Room::lastCommonReadMessage() const
{
    return m_lastCommonReadMessage;
}

void Room::setLastCommonReadMessage(int lastCommonReadMessage)
{
    if (m_lastCommonReadMessage == lastCommonReadMessage) {
        return;
    }
    m_lastCommonReadMessage = lastCommonReadMessage;
    Q_EMIT lastCommonReadMessageChanged();
}

QString Room::lastMessageText() const
{
    return m_lastMessageText;
}

void Room::setLastMessageText(const QString &lastMessageText)
{
    if (m_lastMessageText == lastMessageText) {
        return;
    }
    m_lastMessageText = lastMessageText;
    Q_EMIT lastMessageTextChanged();
}

QString Room::lastMessageAuthor() const
{
    return m_lastMessageAuthor;
}

void Room::setLastMessageAuthor(const QString &lastMessageAuthor)
{
    if (m_lastMessageAuthor == lastMessageAuthor) {
        return;
    }
    m_lastMessageAuthor = lastMessageAuthor;
    Q_EMIT lastMessageAuthorChanged();
}

uint Room::lastMessageTimestamp() const
{
    return m_lastMessageTimestamp;
}

void Room::setLastMessageTimestamp(uint lastMessageTimestamp)
{
    if (m_lastMessageTimestamp == lastMessageTimestamp) {
        return;
    }
    m_lastMessageTimestamp = lastMessageTimestamp;
    Q_EMIT lastMessageTimestampChanged();
}

bool Room::lastMessageIsSystemMessage() const
{
    return m_lastMessageIsSystemMessage;
}

void Room::setLastMessageIsSystemMessage(bool lastMessageIsSystemMessage)
{
    if (m_lastMessageIsSystemMessage == lastMessageIsSystemMessage) {
        return;
    }
    m_lastMessageIsSystemMessage = lastMessageIsSystemMessage;
    Q_EMIT lastMessageIsSystemMessageChanged();
}

bool Room::lastPing() const
{
    return m_lastPing;
}

void Room::setLastPing(bool lastPing)
{
    if (m_lastPing == lastPing) {
        return;
    }
    m_lastPing = lastPing;
    Q_EMIT lastPingChanged();
}

int Room::lastReadMessage() const
{
    return m_lastReadMessage;
}

void Room::setLastReadMessage(int lastReadMessage)
{
    if (m_lastReadMessage == lastReadMessage) {
        return;
    }
    m_lastReadMessage = lastReadMessage;
    Q_EMIT lastReadMessageChanged();
}

int Room::listable() const
{
    return m_listable;
}

void Room::setListable(int listable)
{
    if (m_listable == listable) {
        return;
    }
    m_listable = listable;
    Q_EMIT listableChanged();
}

int Room::lobbyState() const
{
    return m_lobbyState;
}

void Room::setLobbyState(int lobbyState)
{
    if (m_lobbyState == lobbyState) {
        return;
    }
    m_lobbyState = lobbyState;
    Q_EMIT lobbyStateChanged();
}

int Room::lobbyTimer() const
{
    return m_lobbyTimer;
}

void Room::setLobbyTimer(int lobbyTimer)
{
    if (m_lobbyTimer == lobbyTimer) {
        return;
    }
    m_lobbyTimer = lobbyTimer;
    Q_EMIT lobbyTimerChanged();
}

QString Room::name() const
{
    return m_name;
}

void Room::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

int Room::notificationCalls() const
{
    return m_notificationCalls;
}

void Room::setNotificationCalls(int notificationCalls)
{
    if (m_notificationCalls == notificationCalls) {
        return;
    }
    m_notificationCalls = notificationCalls;
    Q_EMIT notificationCallsChanged();
}

int Room::notificationLevel() const
{
    return m_notificationLevel;
}

void Room::setNotificationLevel(int notificationLevel)
{
    if (m_notificationLevel == notificationLevel) {
        return;
    }
    m_notificationLevel = notificationLevel;
    Q_EMIT notificationLevelChanged();
}

QString Room::objectId() const
{
    return m_objectId;
}

void Room::setObjectId(const QString &objectId)
{
    if (m_objectId == objectId) {
        return;
    }
    m_objectId = objectId;
    Q_EMIT objectIdChanged();
}

int Room::participantFlags() const
{
    return m_participantFlags;
}

void Room::setParticipantFlags(int participantFlags)
{
    if (m_participantFlags == participantFlags) {
        return;
    }
    m_participantFlags = participantFlags;
    Q_EMIT participantFlagsChanged();
}

int Room::participantType() const
{
    return m_participantType;
}

void Room::setParticipantType(int participantType)
{
    if (m_participantType == participantType) {
        return;
    }
    m_participantType = participantType;
    Q_EMIT participantTypeChanged();
}

int Room::permissions() const
{
    return m_permissions;
}

void Room::setPermissions(int permissions)
{
    if (m_permissions == permissions) {
        return;
    }
    m_permissions = permissions;
    Q_EMIT permissionsChanged();
}

bool Room::readAll() const
{
    return m_readAll;
}

void Room::setReadAll(bool readAll)
{
    if (m_readAll == readAll) {
        return;
    }
    m_readAll = readAll;
    Q_EMIT readAllChanged();
}

QString Room::sessionId() const
{
    return m_sessionId;
}

void Room::setSessionId(const QString &sessionId)
{
    if (m_sessionId == sessionId) {
        return;
    }
    m_sessionId = sessionId;
    Q_EMIT sessionIdChanged();
}

bool Room::sipEnabled() const
{
    return m_sipEnabled;
}

void Room::setSipEnabled(bool sipEnabled)
{
    if (m_sipEnabled == sipEnabled) {
        return;
    }
    m_sipEnabled = sipEnabled;
    Q_EMIT sipEnabledChanged();
}

QString Room::token() const
{
    return m_token;
}

void Room::setToken(const QString &token)
{
    if (m_token == token) {
        return;
    }
    m_token = token;
    Q_EMIT tokenChanged();
}

Room::Types Room::type() const
{
    return m_type;
}

void Room::setType(Room::Types type)
{
    if (m_type == type) {
        return;
    }
    m_type = type;
    Q_EMIT typeChanged();
}

bool Room::unreadMention() const
{
    return m_unreadMention;
}

void Room::setUnreadMention(bool unreadMention)
{
    if (m_unreadMention == unreadMention) {
        return;
    }
    m_unreadMention = unreadMention;
    Q_EMIT unreadMentionChanged();
}

bool Room::unreadMentionDirect() const
{
    return m_unreadMentionDirect;
}

void Room::setUnreadMentionDirect(bool unreadMentionDirect)
{
    if (m_unreadMentionDirect == unreadMentionDirect) {
        return;
    }
    m_unreadMentionDirect = unreadMentionDirect;
    Q_EMIT unreadMentionDirectChanged();
}

int Room::unreadMessages() const
{
    return m_unreadMessages;
}

void Room::setUnreadMessages(int unreadMessages)
{
    if (m_unreadMessages == unreadMessages) {
        return;
    }
    m_unreadMessages = unreadMessages;
    Q_EMIT unreadMessagesChanged();
}

bool Room::operator ==(const Room &toCompare) const {
    return toCompare.account() == m_account
            && toCompare.token() == token();
}

