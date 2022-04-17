// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QJsonObject>
#include <qjsonobject.h>
#include "nextcloudaccount.h"

/// A talk conversation
class Room : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString actorId READ actorId WRITE setActorId NOTIFY actorIdChanged);
    Q_PROPERTY(QString actorType READ actorType WRITE setActorType NOTIFY actorTypeChanged);
    Q_PROPERTY(int attendeeId READ attendeeId WRITE setAttendeeId NOTIFY attendeeIdChanged);
    Q_PROPERTY(int attendeePermissions READ attendeePermissions WRITE setAttendeePermissions NOTIFY attendeePermissionsChanged);
    Q_PROPERTY(QString attendeePin READ attendeePin WRITE setAttendeePin NOTIFY attendeePinChanged);
    Q_PROPERTY(int callFlag READ callFlag WRITE setCallFlag NOTIFY callFlagChanged);
    Q_PROPERTY(int callPermissions READ callPermissions WRITE setCallPermissions NOTIFY callPermissionsChanged);
    Q_PROPERTY(bool canDeleteConversation READ canDeleteConversation WRITE setCanDeleteConversation NOTIFY canDeleteConversationChanged);
    Q_PROPERTY(bool canEnableSIP READ canEnableSIP WRITE setCanEnableSIP NOTIFY canEnableSIPChanged);
    Q_PROPERTY(bool canLeaveConversation READ canLeaveConversation WRITE setCanLeaveConversation NOTIFY canLeaveConversationChanged);
    Q_PROPERTY(bool canStartCall READ canStartCall WRITE setCanStartCall NOTIFY canStartCallChanged);
    Q_PROPERTY(int defaultPermissions READ defaultPermissions WRITE setDefaultPermissions NOTIFY defaultPermissionsChanged);
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged);
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged);
    Q_PROPERTY(bool hasCall READ hasCall WRITE setHasCall NOTIFY hasCallChanged);
    Q_PROPERTY(bool hasPassword READ hasPassword WRITE setHasPassword NOTIFY hasPasswordChanged);
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged);
    Q_PROPERTY(bool isFavorite READ isFavorite WRITE setIsFavorite NOTIFY isFavoriteChanged);
    Q_PROPERTY(int lastActivity READ lastActivity WRITE setLastActivity NOTIFY lastActivityChanged);
    Q_PROPERTY(int lastCommonReadMessage READ lastCommonReadMessage WRITE setLastCommonReadMessage NOTIFY lastCommonReadMessageChanged);
    Q_PROPERTY(QString lastMessageText READ lastMessageText WRITE setLastMessageText NOTIFY lastMessageTextChanged);
    Q_PROPERTY(QString lastMessageAuthor READ lastMessageAuthor WRITE setLastMessageAuthor NOTIFY lastMessageAuthorChanged);
    Q_PROPERTY(uint lastMessageTimestamp READ lastMessageTimestamp WRITE setLastMessageTimestamp NOTIFY lastMessageTimestampChanged);
    Q_PROPERTY(bool lastMessageIsSystemMessage READ lastMessageIsSystemMessage WRITE setLastMessageIsSystemMessage NOTIFY lastMessageIsSystemMessageChanged);
    Q_PROPERTY(bool lastPing READ lastPing WRITE setLastPing NOTIFY lastPingChanged);
    Q_PROPERTY(int lastReadMessage READ lastReadMessage WRITE setLastReadMessage NOTIFY lastReadMessageChanged);
    Q_PROPERTY(int listable READ listable WRITE setListable NOTIFY listableChanged);
    Q_PROPERTY(int lobbyState READ lobbyState WRITE setLobbyState NOTIFY lobbyStateChanged);
    Q_PROPERTY(int lobbyTimer READ lobbyTimer WRITE setLobbyTimer NOTIFY lobbyTimerChanged);
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(int notificationCalls READ notificationCalls WRITE setNotificationCalls NOTIFY notificationCallsChanged);
    Q_PROPERTY(int notificationLevel READ notificationLevel WRITE setNotificationLevel NOTIFY notificationLevelChanged);
    Q_PROPERTY(QString objectId READ objectId WRITE setObjectId NOTIFY objectIdChanged);
    Q_PROPERTY(int participantFlags READ participantFlags WRITE setParticipantFlags NOTIFY participantFlagsChanged);
    Q_PROPERTY(int participantType READ participantType WRITE setParticipantType NOTIFY participantTypeChanged);
    Q_PROPERTY(int permissions READ permissions WRITE setPermissions NOTIFY permissionsChanged);
    Q_PROPERTY(bool readAll READ readAll WRITE setReadAll NOTIFY readAllChanged);
    Q_PROPERTY(QString sessionId READ sessionId WRITE setSessionId NOTIFY sessionIdChanged);
    Q_PROPERTY(bool sipEnabled READ sipEnabled WRITE setSipEnabled NOTIFY sipEnabledChanged);
    Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged);
    Q_PROPERTY(Room::Types type READ type WRITE setType NOTIFY typeChanged);
    Q_PROPERTY(bool unreadMention READ unreadMention WRITE setUnreadMention NOTIFY unreadMentionChanged);
    Q_PROPERTY(bool unreadMentionDirect READ unreadMentionDirect WRITE setUnreadMentionDirect NOTIFY unreadMentionDirectChanged);
    Q_PROPERTY(int unreadMessages READ unreadMessages WRITE setUnreadMessages NOTIFY unreadMessagesChanged);


public:
    enum Types {
        Unknown = -1,
        OneToOne = 1,
        Group = 2,
        Public = 3,
        Changelog = 4,
    };
    Q_ENUM(Types);

    Room(NextcloudAccount *account, const QJsonObject &obj);
    const NextcloudAccount *account() const;
    void updateFromJsonObject(const QJsonObject &obj);
    bool operator ==(const Room &toCompare) const;

    Q_INVOKABLE void toggleFavorite();

    QString actorId() const;
    QString actorType() const;
    int attendeeId() const;
    int attendeePermissions() const;
    QString attendeePin() const;
    int callFlag() const;
    int callPermissions() const;
    bool canDeleteConversation() const;
    bool canEnableSIP() const;
    bool canLeaveConversation() const;
    bool canStartCall() const;
    int defaultPermissions() const;
    QString description() const;
    QString displayName() const;
    bool hasCall() const;
    bool hasPassword() const;
    int id() const;
    bool isFavorite() const;
    int lastActivity() const;
    int lastCommonReadMessage() const;
    QString lastMessageText() const;
    QString lastMessageAuthor() const;
    uint lastMessageTimestamp() const;
    bool lastMessageIsSystemMessage() const;
    bool lastPing() const;
    int lastReadMessage() const;
    int listable() const;
    int lobbyState() const;
    int lobbyTimer() const;
    QString name() const;
    int notificationCalls() const;
    int notificationLevel() const;
    QString objectId() const;
    int participantFlags() const;
    int participantType() const;
    int permissions() const;
    bool readAll() const;
    QString sessionId() const;
    bool sipEnabled() const;
    QString token() const;
    Room::Types type() const;
    bool unreadMention() const;
    bool unreadMentionDirect() const;
    int unreadMessages() const;

    void setActorId(const QString &actorId);
    void setActorType(const QString &actorType);
    void setAttendeeId(int attendeeId);
    void setAttendeePermissions(int attendeePermissions);
    void setAttendeePin(const QString &attendeePin);
    void setCallFlag(int callFlag);
    void setCallPermissions(int callPermissions);
    void setCanDeleteConversation(bool canDeleteConversation);
    void setCanEnableSIP(bool canEnableSIP);
    void setCanLeaveConversation(bool canLeaveConversation);
    void setCanStartCall(bool canStartCall);
    void setDefaultPermissions(int defaultPermissions);
    void setDescription(const QString &description);
    void setDisplayName(const QString &displayName);
    void setHasCall(bool hasCall);
    void setHasPassword(bool hasPassword);
    void setId(int id);
    void setIsFavorite(bool isFavorite);
    void setLastActivity(int lastActivity);
    void setLastCommonReadMessage(int lastCommonReadMessage);
    void setLastMessageText(const QString &lastMessageText);
    void setLastMessageAuthor(const QString &lastMessageAuthor);
    void setLastMessageTimestamp(uint lastMessageTimestamp);
    void setLastMessageIsSystemMessage(bool lastMessageIsSystemMessage);
    void setLastPing(bool lastPing);
    void setLastReadMessage(int lastReadMessage);
    void setListable(int listable);
    void setLobbyState(int lobbyState);
    void setLobbyTimer(int lobbyTimer);
    void setName(const QString &name);
    void setNotificationCalls(int notificationCalls);
    void setNotificationLevel(int notificationLevel);
    void setObjectId(const QString &objectId);
    void setParticipantFlags(int participantFlags);
    void setParticipantType(int participantType);
    void setPermissions(int permissions);
    void setReadAll(bool readAll);
    void setSessionId(const QString &sessionId);
    void setSipEnabled(bool sipEnabled);
    void setToken(const QString &token);
    void setType(Room::Types type);
    void setUnreadMention(bool unreadMention);
    void setUnreadMentionDirect(bool unreadMentionDirect);
    void setUnreadMessages(int unreadMessages);

Q_SIGNALS:
    void actorIdChanged();
    void actorTypeChanged();
    void attendeeIdChanged();
    void attendeePermissionsChanged();
    void attendeePinChanged();
    void callFlagChanged();
    void callPermissionsChanged();
    void canDeleteConversationChanged();
    void canEnableSIPChanged();
    void canLeaveConversationChanged();
    void canStartCallChanged();
    void defaultPermissionsChanged();
    void descriptionChanged();
    void displayNameChanged();
    void hasCallChanged();
    void hasPasswordChanged();
    void idChanged();
    void isFavoriteChanged();
    void lastActivityChanged();
    void lastCommonReadMessageChanged();
    void lastMessageTextChanged();
    void lastMessageAuthorChanged();
    void lastMessageTimestampChanged();
    void lastMessageIsSystemMessageChanged();
    void lastPingChanged();
    void lastReadMessageChanged();
    void listableChanged();
    void lobbyStateChanged();
    void lobbyTimerChanged();
    void nameChanged();
    void notificationCallsChanged();
    void notificationLevelChanged();
    void objectIdChanged();
    void participantFlagsChanged();
    void participantTypeChanged();
    void permissionsChanged();
    void readAllChanged();
    void sessionIdChanged();
    void sipEnabledChanged();
    void tokenChanged();
    void typeChanged();
    void unreadMentionChanged();
    void unreadMentionDirectChanged();
    void unreadMessagesChanged();

private:
    NextcloudAccount *m_account = nullptr;
    QString m_actorId;
    QString m_actorType;
    int m_attendeeId;
    int m_attendeePermissions;
    QString m_attendeePin;
    int m_callFlag;
    int m_callPermissions;
    bool m_canDeleteConversation;
    bool m_canEnableSIP;
    bool m_canLeaveConversation;
    bool m_canStartCall;
    int m_defaultPermissions;
    QString m_description;
    QString m_displayName;
    bool m_hasCall;
    bool m_hasPassword;
    int m_id;
    bool m_isFavorite;
    int m_lastActivity;
    int m_lastCommonReadMessage;
    QString m_lastMessageText;
    QString m_lastMessageAuthor;
    uint m_lastMessageTimestamp;
    bool m_lastMessageIsSystemMessage;
    bool m_lastPing;
    int m_lastReadMessage;
    int m_listable;
    int m_lobbyState;
    int m_lobbyTimer;
    QString m_name;
    int m_notificationCalls;
    int m_notificationLevel;
    QString m_objectId;
    int m_participantFlags;
    int m_participantType;
    int m_permissions;
    bool m_readAll;
    QString m_sessionId;
    bool m_sipEnabled;
    QString m_token;
    Room::Types m_type;
    bool m_unreadMention;
    bool m_unreadMentionDirect;
    int m_unreadMessages;
};
