#include "roomservice.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMetaMethod>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <qdebug.h>
#include "services/requestfactory.h"
#include "services/capabilities.h"

RoomService::RoomService(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(m_accountService, &QAbstractItemModel::modelReset, this, &RoomService::onAccountsChanged);
    connect(m_accountService, &QAbstractItemModel::rowsRemoved, this, &RoomService::onAccountsChanged);
    connect(m_accountService, &QAbstractItemModel::dataChanged, this, &RoomService::onAccountUpdated);
}

int RoomService::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
   return m_rooms.length();
}

QVariant RoomService::data(const QModelIndex &index, int role) const
{
    Room *room = m_rooms[index.row()];
    switch (role) {
        case NameRole:
            return room->name();
        case TokenRole:
            return room->token();
        case AccountRole:
            return room->account().id();
        case UserIdRole:
            return room->account().userId();
        case UnreadRole:
            return room->unreadMessages();
        case MentionedRole:
            return room->unreadMention();
        case ColorRole:
            if (room->account().colorOverride().isValid()) {
                return room->account().colorOverride();
            }
            return room->account().capabilities()->primaryColor();
        case LastMessageTextRole:
            return room->lastMessageText();
        case LastMessageAuthorRole:
            return room->lastMessageAuthor();
        case LastMessageTimestampRole:
            return room->lastMessageTimestamp();
        case LastMessageIsSystemMessageRole:
            return room->lastMessageIsSystemMessage();
        case TypeRole:
            return room->type();
        case ConversationNameRole:
            return room->conversationName();
    }
    return {};
}

QHash<int, QByteArray> RoomService::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TokenRole] = "token";
    roles[AccountRole] = "accountId";
    roles[UserIdRole] = "accountUserId";
    roles[UnreadRole] = "unreadMessages";
    roles[MentionedRole] = "unreadMention";
    roles[ColorRole] = "primaryColor";
    roles[LastMessageTextRole] = "lastMessageText";
    roles[LastMessageAuthorRole] = "lastMessageAuthor";
    roles[LastMessageTimestampRole] = "lastMessageTimestamp";
    roles[LastMessageIsSystemMessageRole] = "lastMessageIsSystemMessage";
    roles[TypeRole] = "conversationType";
    roles[ConversationNameRole] = "conversationName";
    return roles;
}

void RoomService::loadRooms() {
    if(
        isSignalConnected(QMetaMethod::fromSignal(&QNetworkAccessManager::finished))
    ) {
        foreach (QNetworkReply* reply, m_rooms_requests) {
            reply->abort();
        }
        disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
        qDebug() << "client timeout for previous loadRooms request";
    } else if (m_nam.networkAccessible() == QNetworkAccessManager::NotAccessible) {
        qDebug() << "no network, waiting";
        return;
    }
    m_pendingRequests = m_accountService->getAccounts().length();
    if(m_pendingRequests > 0) {
        connect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
    }
    foreach (NextcloudAccount* account, m_accountService->getAccounts()) {
        if(account->password().isEmpty()) {
            m_pendingRequests--;
            continue;
        } else if (!account->capabilities()->areAvailable()) {
            account->capabilities()->request();
            m_pendingRequests--;
            continue;
        }
        // room endpoint carries talk specific indicator whether capabilities have changed
        connect(&m_nam, &QNetworkAccessManager::finished, account->capabilities(), &Capabilities::checkTalkCapHash);

        QUrl endpoint = QUrl(account->host());
        QString apiV = "v" + QString::number(account->capabilities()->getConversationApiLevel());
        endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/" + apiV + "/room");
        endpoint.setQuery("format=json");
        QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
        QNetworkReply* reply = m_nam.get(request);
        m_rooms_requests.append(reply);
        reply->setProperty("AccountID", account->id());
    }
}

void RoomService::roomsLoadedFromAccount(QNetworkReply *reply) {
    m_pendingRequests--;
    if(m_pendingRequests == 0) {
        disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
    }

    if(m_rooms_requests.contains(reply)) {
        m_rooms_requests.removeOne(reply);
    }

    switch (reply->error()) {
        case QNetworkReply::NetworkSessionFailedError:
            qDeleteAll(m_nam.findChildren<QNetworkReply *>());
            disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomsLoadedFromAccount);
            m_pendingRequests = 0;
            return;
        default:
            break;
    }

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "network issue or unauthed, code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(m_nam.networkAccessible() != QNetworkAccessManager::Accessible) {
            qDebug() << "Network not accessible";
        }

        return;
    }

    const NextcloudAccount* currentAccount = nullptr;
    int accounts = m_accountService->getAccounts().length();
    for(int i = 0; i < accounts; i++) {
        const NextcloudAccount* account = m_accountService->getAccounts().at(i);
        if(account->host().authority() == reply->url().authority()) {
            currentAccount = account;
            break;
        }
    }
    if(currentAccount == nullptr) {
        qDebug() << "Coult not figure out account for this result set";
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    qDebug() << apiResult;
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    // qDebug() << "JSON" << payload;
    QJsonObject meta = root.find("meta").value().toObject();
    QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt();
        if(statuscode.toInt() == 0) {
            qDebug() << "payload was " << payload;
            qDebug() << "url was" << reply->url();
        }
        return;
    }

    const QJsonArray data = root.find("data").value().toArray();
    for (const QJsonValue& value : data) {
        QJsonObject room = value.toObject();
        Room *model = new Room;
        model
                ->setAccount(currentAccount)
                ->setConversationName(room.value("name").toString())
                ->setName(room.value("displayName").toString())
                ->setFavorite(room.value("isFavorite").toBool())
                ->setHasPassword(room.value("hasPassword").toBool())
                ->setToken(room.value("token").toString())
                ->setType(static_cast<Room::RoomType>(room.value("type").toInt()))
                ->setUnreadMention(room.value("unreadMention").toBool())
                ->setUnreadMessages(room.value("unreadMessages").toInt())
                ->setLastActivity(room.value("lastActivity").toInt());

        if(room.contains("lastMessage")) {
            QJsonObject lastMessage = room.value("lastMessage").toObject();
            QString message = lastMessage.value("message").toString();
            QJsonObject parameters = lastMessage.value("messageParameters").toObject();

            foreach(QString placeholder, parameters.keys())
            {
                QString name = parameters.value(placeholder).toObject().value("name").toString();
                message.replace("{" + placeholder + "}", name, Qt::CaseSensitive);
            }

            model->setLastMessage(
                message,
                lastMessage.value("actorDisplayName").toString(),
                lastMessage.value("timestamp").toVariant().toUInt(),
                lastMessage.value("systemMessage").toString() != ""
            );
        }

        Room *knownRoom = findRoomByTokenAndAccount(model->token(), model->account().id());
        if (knownRoom) {
            int i = m_rooms.indexOf(knownRoom);
            m_rooms.replace(i, model);
        } else {
            beginInsertRows(QModelIndex(), m_rooms.length(), m_rooms.length());
            m_rooms.append(model);
            endInsertRows();
        }
    }

    std::sort(m_rooms.begin(), m_rooms.end(), [](const Room *a, const Room *b) {
        return a->lastActivity() > b->lastActivity();
    });
    dataChanged(index(0), index(m_rooms.length() - 1));
}

Room *RoomService::getRoom(const QString &token, int accountId) const {
    for (auto room : m_rooms) {
        if(room->token() == token && room->account().id() == accountId) {
            return room;
        }
    }
    return nullptr;
}

void RoomService::startPolling(const QString &token, int accountId) {
    activeToken = token;
    activeAccountId = accountId;
    m_isPolling = true;
    m_lookIntoFuture = 0;
    pollRoom();
    emitAfterActiveRoomChanged(token, accountId);
}

void RoomService::emitAfterActiveRoomChanged(const QString &token, int accountId) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage event = QDBusMessage::createSignal("/conversation", "org.nextcloud.talk", "afterActiveConversationChanged");
    event << token << accountId;
    bus.send(event);
}

bool RoomService::isPolling(const QString &token, int accountId) {
    return activeToken == token
            && activeAccountId == accountId
            && m_isPolling;
}

void RoomService::stopPolling() {
    m_isPolling = false;
}

Room *RoomService::findRoomByTokenAndAccount(const QString &token, const int accountId) const {
    for (auto room : m_rooms) {
        if(room->token() == token && room->account().id() == accountId) {
            return room;
        }
    }
    return nullptr;
}

void RoomService::pollRoom() {
    if(!m_isPolling) {
        return;
    }
    NextcloudAccount* account = m_accountService->getAccountById(activeAccountId);
    if (!account) {
        qDebug() << "Failed to poll for room" << activeAccountId;
        return;
    }
    connect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomPolled);
    int lastKnownMessageId = m_db.lastKnownMessageId(activeAccountId, activeToken);
    QString includeLastKnown = m_lookIntoFuture == 0 ? "1" : "0";
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/chat/" + activeToken);
    endpoint.setQuery("format=json&lookIntoFuture=" + QString::number(m_lookIntoFuture) +
                      "&timeout=30&lastKnownMessageId=" + QString::number(lastKnownMessageId) +
                      "&includeLastKnown=" + includeLastKnown);

    QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
    m_nam.get(request);
}

void RoomService::roomPolled(QNetworkReply *reply) {
    qDebug() << "polling for new messages finished " << reply->error();
    qDebug() << "status code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    disconnect(&m_nam, &QNetworkAccessManager::finished, this, &RoomService::roomPolled);

    if(!m_isPolling) {
        return;
    }

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 304) {
        qDebug() << "no new messages";
        pollRoom();
        return;
    }

    if(reply->error() == QNetworkReply::ContentNotFoundError) {
        qDebug() << "Some server error?! check logs! Polling stopped.";
        return;
    }

    if(reply->error() == QNetworkReply::TimeoutError) {
        qDebug() << "timeout…";
        pollRoom();
        return;
    }

    if(reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qDebug() << "issue occured:" << reply->errorString() << "→ Polling stopped!";
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    //qDebug() << "JSON" << payload;
    QJsonObject meta = root.find("meta").value().toObject();
    QJsonValue statuscode = meta.find("statuscode").value();
    if(statuscode.toInt() != 200) {
        qDebug() << "unexpected OCS code " << statuscode.toInt() << "→ polling stopped";
        return;
    }

    QJsonArray data = root.find("data").value().toArray();
    int start, end, step;
    if(m_lookIntoFuture == 0) {
        start = data.size() - 1;
        end = -1;
        step = -1;
        // this happens just once to get some history
        m_lookIntoFuture = 1;
    } else {
        start = 0;
        end = data.size();
        step = 1;
    }

    for(auto i = start; i != end; i += step) {
        QJsonValue value = data.at(i);
        QJsonObject messageData = value.toObject();
        int msgId = messageData.value("id").toInt();
        if(msgId > m_db.lastKnownMessageId(activeAccountId, activeToken)) {
            // do not lower value when we fetched history
            m_db.setLastKnownMessageId(activeAccountId, activeToken, msgId);
        }

        QString systemMessage = messageData.value("systemMessage").toString();
        if(systemMessage == "call_left"
           || systemMessage == "call_started"
           || systemMessage == "conversation_created"
        ) {
            // some system message we simply ignore:
            // - created because maybe it is not so important
            // - calls because they are not supported
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromVariant(value.toVariant());
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //qDebug() << strJson;
        emit newMessage(strJson);
    }

    pollRoom();
}

void RoomService::sendMessage(const QString &messageText, int replyToId) {
    NextcloudAccount* account = m_accountService->getAccountById(activeAccountId);
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/v1/chat/" + activeToken);

    QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray payload = QString("message=" + QUrl::toPercentEncoding(messageText)).toUtf8();
    if(replyToId > -1) {
        payload += QString("&replyTo=" + QString::number(replyToId)).toUtf8();
    }

    namPosting.post(request, payload);
}

void RoomService::onAccountsChanged() {
    qDebug() << "RoomService acting on Accounts change";
    beginResetModel();
    qDeleteAll(m_rooms);
    m_rooms.clear();
    endResetModel();
    loadRooms();
}

void RoomService::onAccountUpdated() {
    qDebug() << "RoomService acting on Account update";
    beginResetModel();
    endResetModel();
}
