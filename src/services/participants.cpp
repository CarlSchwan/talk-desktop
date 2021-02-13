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
        return QVariant(m_participants[index.row()].presence );
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
    roles[PresenceRole] = "PresenceStatus";
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
    QString apiV = m_activeAccount->capabilities()->hasConversationV2() ? "v2" : "v1";
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
    QHash<QString, Participants::PresenceStatus> statusMap;
    statusMap["offline"]   = Participants::PresenceStatus::StatusOffline;
    statusMap["online"]    = Participants::PresenceStatus::StatusOnline;
    statusMap["away"]      = Participants::PresenceStatus::StatusAway;
    statusMap["dnd"]       = Participants::PresenceStatus::StatusDnD;
    statusMap["invisible"] = Participants::PresenceStatus::StatusInvisible;

    int checkId = std::time(0);

    QJsonArray data = root.find("data").value().toArray();
    foreach(const QJsonValue& value, data) {
        QJsonObject participantData = value.toObject();
        Participant model = Participant(
            participantData.value("userId").toString(),
            participantData.value("displayName").toString(),
            participantData.value("participantType").toInt(),
            participantData.value("lastPing").toInt(),
            participantData.value("sessionId").toString()
        );

        if (participantData.contains("status")) {
            model.presence = statusMap.value(participantData.value("status").toString(), Participants::PresenceStatus::StatusOnline);
        } else {
            model.presence = Participants::PresenceStatus::StatusOffline;
        }

        if(participantData.contains("inCall")) {
            model.inCall = participantData.value("inCall").toInt();
        }
        model._checkId = checkId;

        int i = findParticipant(model.userId);
        if(i >= 0) {
            m_participants.replace(i, model);
            dataChanged(index(i), index(i));
        } else {
            beginInsertRows(QModelIndex(), m_participants.length(), m_participants.length());
            m_participants.append(model);
            endInsertRows();
        }
    }
    removeParticipants(checkId);
}

void Participants::removeParticipants(int checkId)
{
    QVector<Participant>::const_iterator i;
    for(i = m_participants.begin(); i != m_participants.end(); i++) {
        if(i->_checkId != checkId) {
            //int j = m_participants.indexOf(*i);
            //beginRemoveRows(QModelIndex(), j, j);
            //m_participants.removeAt(j);
            //endRemoveRows();
            m_participants.removeOne(*i);
        }
    }
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
