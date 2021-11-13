// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../constants/PresenceStatusClass.h"
#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NextcloudAccount;

class ParticipantModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ParticipantModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRoom(const QString &token, NextcloudAccount *account);

    enum ParticipantRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        StatusRole,
        PresenceRole,
        StatusIconRole,
        StatusMessageRole,
        ModeratorRole,
        AvatarRole,
    };

    struct Participant {
        Participant()
        {
        }
        Participant(QString userId,
                    QString displayName,
                    int type,
                    int ping,
                    QString sessionId,
                    bool inCall = false,
                    PresenceStatus presence = PresenceStatus::Offline,
                    QString statusIcon = QString(),
                    QString statusMessage = QString())
        {
            this->userId = userId;
            this->displayName = displayName;
            this->type = type;
            this->ping = ping;
            this->sessionId = sessionId;
            this->inCall = inCall;
            this->presence = presence;
            this->statusIcon = statusIcon;
            this->statusMessage = statusMessage;
        }

        QString userId;
        QString displayName;
        int type;
        int ping;
        QString sessionId;
        int inCall = 0;
        PresenceStatus presence;
        QString statusIcon;
        QString statusMessage;
        int _checkId;

        bool operator==(const Participant &toCompare) const
        {
            return userId == toCompare.userId;
        }

        bool diverts(const Participant &toCompare) const
        {
            return userId != toCompare.userId || displayName != toCompare.displayName || type != toCompare.type || sessionId != toCompare.sessionId
                || presence != toCompare.presence || statusIcon != toCompare.statusIcon || statusMessage != toCompare.statusMessage;
        }
    };

public slots:
    void pullParticipantModel();

private slots:
    void participantsPulled(QNetworkReply *reply);

private:
    NextcloudAccount *m_activeAccount;
    QVector<Participant> m_participants;
    PresenceStatus presence;
    QString m_token;

    int findParticipant(const QString &userId);
    int removeParticipantModel(int checkId);
    bool isModerator(const Participant &participant) const;
};
