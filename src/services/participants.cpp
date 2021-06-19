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
    if (parent.isValid())
        return 0;

    return m_participants.length();
}

QVariant Participants::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case IdRole:
        return QVariant(m_participants[index.row()].userId);
    case NameRole:
        return QVariant(m_participants[index.row()].displayName);
    case TypeRole:
        return QVariant(m_participants[index.row()].type);
    case StatusRole:
        return QVariant(m_participants[index.row()].sessionId != "0");
    case PresenceRole:
        return QVariant(m_participants[index.row()].presence);
    case StatusIconRole:
        return QVariant(m_participants[index.row()].statusIcon);
    case StatusMessageRole:
        return QVariant(m_participants[index.row()].statusMessage);
    case ModeratorRole:
        return QVariant(isModerator(m_participants[index.row()]));
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Participants::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "userId";
    roles[NameRole] = "displayName";
    roles[TypeRole] = "participantType";
    roles[StatusRole] = "isOnline";
    roles[PresenceRole] = "presenceStatus";
    roles[StatusIconRole] = "statusIcon";
    roles[StatusMessageRole] = "statusMessage";
    roles[ModeratorRole] = "isModerator";
    return roles;
}


void Participants::pullParticipants(QString token, int accountId)
{
    try {
        m_activeAccount = m_accountService->getAccountById(accountId);
    } catch (QException &e) {
        Q_UNUSED(e)
        qDebug() << "Failed to pull participants for room" << accountId;
        return;
    }
    if(!isSignalConnected(QMetaMethod::fromSignal(&QNetworkAccessManager::finished))) {
        connect(&m_nam, &QNetworkAccessManager::finished, this, &Participants::participantsPulled);
    } else {
        m_reply->abort();
    }
    QUrl endpoint = QUrl(m_activeAccount->host());
    QString apiV = "v" + QString::number(m_activeAccount->capabilities()->getConversationApiLevel());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/" + apiV+ "/room/" + token + "/participants");
    QUrlQuery q(endpoint);
    q.addQueryItem("format", "json");
    q.addQueryItem("includeStatus", "true");
    endpoint.setQuery(q);

    QNetworkRequest request = RequestFactory::getRequest(endpoint, m_activeAccount);
    m_reply = m_nam.get(request);
}

void Participants::participantsPulled(QNetworkReply *reply)
{
    disconnect(&m_nam, &QNetworkAccessManager::finished, this, &Participants::participantsPulled);

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed, code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    qDebug() << "Participant JSON" << payload;

    QJsonObject meta = root.find("meta").value().toObject();
    QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        if(statuscode.toInt() != 200) {
            qDebug() << "payload was " << payload;
            qDebug() << "url was" << reply->url();
        }
        return;
    }

    // https://github.com/nextcloud/server/blob/master/lib/public/UserStatus/IUserStatus.php#L42-L66
    QHash<QString, PresenceStatus> statusMap;
    statusMap["offline"]   = PresenceStatus::Offline;
    statusMap["online"]    = PresenceStatus::Online;
    statusMap["away"]      = PresenceStatus::Away;
    statusMap["dnd"]       = PresenceStatus::DND;
    statusMap["invisible"] = PresenceStatus::Invisible;

    int checkId = std::time(nullptr);

    bool isDirty = m_participants.empty();

    QJsonArray data = root.find("data").value().toArray();
    foreach(const QJsonValue& value, data) {
        QJsonObject participantData = value.toObject();

        int apiLevel = m_activeAccount->capabilities()->getConversationApiLevel();

        QString userId = apiLevel == 4 ? participantData.value("actorId").toString() : participantData.value("userId").toString();
        // it is only used to indicate whether a participant is present in a room, so any string not "0" suffices
        QString sessionId = apiLevel < 4
                ? participantData.value("sessionId").toString()
                : (participantData.value("sessionIds").toArray().empty()
                   ? "0"
                   : "yes");

        Participant model = Participant(
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

int Participants::findParticipant(QString userId)
{
    QVector<Participant>::iterator i;
    for(i = m_participants.begin(); i != m_participants.end(); i++) {
        if(i->userId == userId) {
            return m_participants.indexOf(*i);
        }
    }
    return -1;
}

bool Participants::isModerator(const Participant participant) const
{
    return participant.type == 1 || participant.type == 2 || participant.type == 6;
}
