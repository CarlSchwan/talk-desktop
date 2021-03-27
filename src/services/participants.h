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
    QHash<int, QByteArray> roleNames() const;

    enum ParticipantRoles {
        IdRole = Qt::UserRole + 1,
        NameRole = Qt::UserRole + 2,
        TypeRole = Qt::UserRole + 3,
        StatusRole = Qt::UserRole + 4,
        PresenceRole = Qt::UserRole + 5,
    };

    struct Participant
    {
        Participant() {}
        Participant(QString userId, QString displayName, int type, int ping, QString sessionId, bool inCall = false, PresenceStatus presence = PresenceStatus::Offline)
        {
            this->userId = userId;
            this->displayName = displayName;
            this->type = type;
            this->ping = ping;
            this->sessionId = sessionId;
            this->inCall = inCall;
            this->presence = presence;
        }

        QString userId;
        QString displayName;
        int type;
        int ping;
        QString sessionId;
        int inCall = 0;
        PresenceStatus presence;
        int _checkId;

        bool operator==(const Participant& toCompare) const
        {
            return userId == toCompare.userId;
        }
    };

public slots:
    void pullParticipants(QString token, int accountId);

private slots:
    void participantsPulled(QNetworkReply *reply);

private:
    Accounts* m_accountService = Accounts::getInstance();
    NextcloudAccount* m_activeAccount;
    QNetworkAccessManager m_nam;
    QNetworkReply* m_reply;
    QVector<Participant> m_participants;

    int findParticipant(QString userId);
    void removeParticipants(int checkId);
    PresenceStatus presence;
};

#endif // PARTICIPANTS_H
