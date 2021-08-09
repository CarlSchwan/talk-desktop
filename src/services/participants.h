#ifndef PARTICIPANTS_H
#define PARTICIPANTS_H

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "accounts.h"
#include "../constants/PresenceStatusClass.h"

class Participants : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit Participants(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum ParticipantRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        StatusRole,
        PresenceRole,
        StatusIconRole,
        StatusMessageRole,
        ModeratorRole
    };

    struct Participant
    {
        Participant() {}
        Participant(
                QString userId,
                QString displayName,
                int type,
                int ping,
                QString sessionId,
                bool inCall = false,
                PresenceStatus presence = PresenceStatus::Offline,
                QString statusIcon = QString(),
                QString statusMessage = QString()
        )
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

        bool operator==(const Participant& toCompare) const
        {
            return userId == toCompare.userId;
        }

        bool diverts(const Participant& toCompare) const
        {
            return userId != toCompare.userId
                || displayName != toCompare.displayName
                || type != toCompare.type
                || sessionId != toCompare.sessionId
                || presence != toCompare.presence
                || statusIcon != toCompare.statusIcon
                || statusMessage != toCompare.statusMessage;
        }
    };

    void setTokenAndAccountId(const QString &token, int accountId);

public slots:
    void pullParticipants();

private slots:
    void participantsPulled(QNetworkReply *reply);

private:
    Accounts* m_accountService = Accounts::getInstance();
    NextcloudAccount* m_activeAccount;
    QNetworkAccessManager m_nam;
    QNetworkReply* m_reply = nullptr;
    QVector<Participant> m_participants;
    PresenceStatus presence;
    QString m_token;
    int m_accountId;

    int findParticipant(QString userId);
    int removeParticipants(int checkId);
    bool isModerator(const Participant participant) const;
};

#endif // PARTICIPANTS_H
