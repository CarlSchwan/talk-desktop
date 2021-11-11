// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <deque>
#include "db.h"

class NextcloudAccount;
class QNetworkReply;

class MessageEventModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MessageType {
        RegularTextMessage,
        SystemMessage,
        SingleLinkGiphyMessage,
        SingleLinkTenorMessage,
        SingleLinkGifMessage,
        SingleLinkMessage,
        SingleLinkVideoMessage,
        SingleLinkImageMessage,
        SingleNcAttachmentMessage,
        SingleNcGeolocationMessage,
        VoiceMessage
    };

    Q_ENUM(MessageType)

    struct Message
    {
        QString text;
        QJsonObject obj;
        MessageType type = MessageType::RegularTextMessage;
    };

    MessageEventModel(QObject *parent = nullptr);
    ~MessageEventModel();

    enum CustomRole {
        MessageRole = Qt::UserRole + 1,
        ActorDisplayNameRole,
        DateRole,
        AvatarRole,
        ShowAuthorRole,
        IsHiglightedRole,
        IsLocalUserRole,
        EventTypeRole,
        FilePreviewUrlRole,
        FileUrlRole,
        ContentTypeRole // minetype
    };

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Called when a new message is fetched.
    void addMessages(const QJsonObject &message);
    void setRoom(const QString &token, int lastReadMessage, NextcloudAccount *account);

    /// Called when the user change room.
    void clear();

    void sendMessage(const QString &messageText, int replyToId);
    void roomPolled(QNetworkReply *reply, const QString &token);

private Q_SLOTS:
    void pollRoom();
    void emitAfterActiveRoomChanged(const QString &token, NextcloudAccount *account);

Q_SIGNALS:
    void pollingDone();

private:
    bool hasFileAttachment(const QJsonObject &messageParameters) const;
    bool hasGeoLocation(const QJsonObject &messageParameters) const;
    QUrl getImageUrl(const QJsonObject &messageParameters) const;

    std::deque<Message> m_messages; // deque allows fast insertion at the end and begining
    NextcloudAccount *m_account = nullptr;
    QString m_token;
    int m_lastReadMessage;
    QString m_localUser;
    int m_lookIntoFuture = 0;
    Db m_db;
};
