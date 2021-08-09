// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messageeventmodel.h"
#include <QDateTime>

MessageEventModel::MessageEventModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

MessageEventModel::~MessageEventModel()
{
}

void MessageEventModel::addMessages(const QJsonObject &obj)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    auto &message = m_messages.emplace_back();
    message.text = obj["message"].toString().replace("{actor}", obj["actorDisplayName"].toString())
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "gt;")
        .replace("\n", "<br />");

    const QJsonObject parameters = obj.value("messageParameters").toObject();

    for (const QString &placeholder: parameters.keys()) {
        const auto subString = "{" + placeholder + "}";
        if (placeholder == "user") {
            message.text = message.text.replace(message.text.indexOf(subString), subString.size(), parameters[placeholder].toObject()["name"].toString());
        } else if (placeholder.startsWith("mention-")) {
            message.text = message.text.replace(message.text.indexOf(subString), subString.size(), "<a href=''>" + parameters[placeholder].toObject()["name"].toString() + "</a>");
        }

        //QString name = parameters.value(placeholder).toObject().value("name").toString();
        //message.text.replace("{" + placeholder + "}", name, Qt::CaseSensitive);
    }

    if (message.text == "{file}") {
        message.type = File;
        message.text = "File TODO";
    }
    message.obj = obj;
    endInsertRows();
}

void MessageEventModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

int MessageEventModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_messages.size();
}

QVariant MessageEventModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &message = m_messages[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            return message.text;
        case ActorDisplayNameRole:
            return message.obj["actorId"].toString();
        case DateRole:
            return QDateTime::fromMSecsSinceEpoch(message.obj["timestamp"].toInt() * 1000);
        case AvatarRole:
            return QStringLiteral("image://avatar/") + QString::number(m_accountId) + "/" + message.obj["actorId"].toString() + "/";
        case ShowAuthorRole: {
            if (index.row() == 0) {
                return true;
            }
            qDebug() << message.obj["actorId"] << index.row() - 1;
            const auto &lastMessage = m_messages[index.row() - 1];
            qDebug() << lastMessage.obj["actorId"] << message.obj["actorId"];
            return lastMessage.obj["actorId"].toString() != message.obj["actorId"].toString();
        }
        case IsHiglightedRole:
            return false; // TODO
        case IsLocalUserRole:
            return false; // TODO
    }

    return {};
}

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {ActorDisplayNameRole, QByteArrayLiteral("actorDisplayName")},
        {DateRole, QByteArrayLiteral("time")},
        {AvatarRole, QByteArrayLiteral("avatar")},
        {ShowAuthorRole, QByteArrayLiteral("showAuthor")},
        {IsHiglightedRole, QByteArrayLiteral("isHighlighted")},
        {IsLocalUserRole, QByteArrayLiteral("isLocalUser")}
    };
}

void MessageEventModel::setAccountId(int accountId)
{
    m_accountId = accountId;
    clear();
}
