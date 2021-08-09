// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <deque>

enum MessageType {
    Text,
    File,
};

struct Message
{
    Message() {};
    QString text;
    QJsonObject obj;
    MessageType type = Text;
};

class MessageEventModel : public QAbstractListModel
{
    Q_OBJECT
public:
    Q_ENUM(MessageType)

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
    };

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Called when a new message is fetched.
    void addMessages(const QJsonObject &message);
    void setAccountId(int accountId);

    /// Called when the user change room.
    void clear();

private:
    std::deque<Message> m_messages; // deque allows fast insertion at the end and begining
    int m_accountId;
    QString m_localUser;
};
