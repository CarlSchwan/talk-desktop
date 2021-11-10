#include <ctime>
#include <QException>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QMetaMethod>
#include "participants.h"
#include "requestfactory.h"
#include "capabilities.h"

Participants::Participants(QObject *parent)
    : QAbstractListModel(parent)
{
}

int Participants::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return m_participants.length();
}

QVariant Participants::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    const auto &participant = m_participants[index.row()];

    switch (role) {
    case IdRole:
        return participant.userId;
    case NameRole:
        return participant.displayName;
    case TypeRole:
        return participant.type;
    case StatusRole:
        return participant.sessionId != "0";
    case PresenceRole:
        return participant.presence;
    case StatusIconRole:
        return participant.statusIcon;
    case StatusMessageRole:
        return participant.statusMessage;
    case ModeratorRole:
        return isModerator(participant);
    case AvatarRole:
        return QLatin1String("image://avatar/") + QString::number(m_activeAccount->id()) + QLatin1Char('/') + participant.userId + QLatin1Char('/');

    default:
        return {};
    }
}

QHash<int, QByteArray> Participants::roleNames() const
{
    return {
        {IdRole, QByteArrayLiteral("userId")},
        {NameRole, QByteArrayLiteral("displayName")},
        {TypeRole, QByteArrayLiteral("participantType")},
        {StatusRole, QByteArrayLiteral("isOnline")},
        {PresenceRole, QByteArrayLiteral("presenceStatus")},
        {StatusIconRole, QByteArrayLiteral("statusIcon")},
        {StatusMessageRole, QByteArrayLiteral("statusMessage")},
        {ModeratorRole, QByteArrayLiteral("isModerator")},
        {AvatarRole, QByteArrayLiteral("avatar")},
    };
}

void Participants::setRoom(const QString &token, NextcloudAccount *account)
{
    m_token = token;
    m_activeAccount = account;
}

void Participants::pullParticipants()
{
    QUrl endpoint = QUrl(m_activeAccount->host());
    const QString apiV = QChar('v') + QString::number(m_activeAccount->capabilities()->getConversationApiLevel());
    endpoint.setPath(endpoint.path() + QStringLiteral("/ocs/v2.php/apps/spreed/api/") + apiV + QStringLiteral("/room/") + m_token + QStringLiteral("/participants"));
    QUrlQuery q(endpoint);
    q.addQueryItem("format", "json");
    q.addQueryItem("includeStatus", "true");
    endpoint.setQuery(q);

    m_activeAccount->get(endpoint, [this](QNetworkReply *reply) {
        participantsPulled(reply);
    });
}

void Participants::participantsPulled(QNetworkReply *reply)
{
    const QByteArray payload = reply->readAll();
    const QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    const QJsonObject q = apiResult.object();
    const QJsonObject root = q.find("ocs").value().toObject();
    // qDebug() << "Participant JSON" << payload;

    const QJsonObject meta = root.find("meta").value().toObject();
    const QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        if(statuscode.toInt() != 200) {
            qDebug() << "payload was " << payload;
            qDebug() << "url was" << reply->url();
        }
        return;
    }

    // https://github.com/nextcloud/server/blob/master/lib/public/UserStatus/IUserStatus.php#L42-L66
    QHash<QString, PresenceStatus> statusMap = {
        {QStringLiteral("offline"), PresenceStatus::Offline},
        {QStringLiteral("online"), PresenceStatus::Online},
        {QStringLiteral("away"), PresenceStatus::Away},
        {QStringLiteral("dnd"), PresenceStatus::DND},
        {QStringLiteral("invisible"), PresenceStatus::Invisible},
    };

    int checkId = std::time(nullptr);

    bool isDirty = m_participants.empty();

    const QJsonArray data = root.find("data").value().toArray();
    for(const QJsonValue &value : data) {
        const QJsonObject participantData = value.toObject();

        int apiLevel = m_activeAccount->capabilities()->getConversationApiLevel();

        const QString userId = apiLevel == 4 ? participantData.value("actorId").toString() : participantData.value("userId").toString();
        // it is only used to indicate whether a participant is present in a room, so any string not "0" suffices
        const QString sessionId = apiLevel < 4
                ? participantData.value("sessionId").toString()
                : (participantData.value("sessionIds").toArray().empty()
                   ? "0"
                   : "yes");

        Participant model(
            userId,
            participantData.value("displayName").toString(),
            participantData.value("participantType").toInt(),
            participantData.value("lastPing").toInt(),
            sessionId
        );

        if (participantData.contains("status")) {
            model.presence = statusMap.value(participantData.value("status").toString(), PresenceStatus::Offline);
        }
        if(participantData.contains("inCall")) {
            model.inCall = participantData.value("inCall").toInt();
        }
        if(participantData.contains("statusIcon")) {
            model.statusIcon = participantData.value("statusIcon").toString("");
        }
        if(participantData.contains("statusMessage")) {
            model.statusMessage = participantData.value("statusMessage").toString("");
        }

        model._checkId = checkId;

        int i = findParticipant(model.userId);
        if(i >= 0) {
            if(model.diverts(m_participants.at(i))) {
                m_participants.replace(i, model);
                isDirty = true;
            } else {
                m_participants[i]._checkId = checkId;
            }
        } else {
            beginInsertRows(QModelIndex(), m_participants.length(), m_participants.length());
            m_participants.append(model);
            endInsertRows();
        }
    }

    isDirty = removeParticipants(checkId) > 0 || isDirty;

    if (isDirty) {
        // TODO sort should be implemented with QSortFilterProxyModel
        std::sort(m_participants.begin(), m_participants.end(), [this](const Participant& a, const Participant b) {
            // sort onliners on top
            if((a.sessionId != "0") != (b.sessionId != "0")) {
                return a.sessionId != "0";
            }

            // prefer moderators
            if (isModerator(a) != isModerator(b)) {
                return isModerator(a);
            }

            //return a.displayName < b.displayName;
            return a.displayName.compare(b.displayName, Qt::CaseInsensitive) < 0;
        });
        dataChanged(index(0), index(m_participants.length() - 1));
    }
}

int Participants::removeParticipants(int checkId)
{
    int removed = 0;

    QVector<Participant>::const_iterator i;
    for(i = m_participants.begin(); i != m_participants.end(); i++) {
        if(i->_checkId != checkId) {
            m_participants.removeOne(*i);
            removed++;
            // Potential crash when the all participants were removed
        }
    }

    return removed;
}

int Participants::findParticipant(const QString &userId)
{
    QVector<Participant>::iterator i;
    for(i = m_participants.begin(); i != m_participants.end(); i++) {
        if(i->userId == userId) {
            return m_participants.indexOf(*i);
        }
    }
    return -1;
}

bool Participants::isModerator(const Participant &participant) const
{
    return participant.type == 1 || participant.type == 2 || participant.type == 6;
}
