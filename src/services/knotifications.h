// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>

#include <KNotification>

#incl
class NotificationsManager : public QObject
{
    Q_OBJECT

public:
    static NotificationsManager &instance();

    Q_INVOKABLE void postNotification(Room *room, const QString &roomName, const QString &sender, const QString &text, const QImage &icon, const QString &replyEventId);

    void watchAccounts(QVector<NextcloudAccount*> accounts);
    void watchAccounts(Accounts* accountService);

private:
    NotificationsManager(QObject *parent = nullptr);
    QVector<NextcloudAccount*> m_accounts;

    QMultiMap<QString, KNotification *> m_notifications;
};
