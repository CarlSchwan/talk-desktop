#include "roomservice.h"
#include <nemonotifications-qt5/notification.h>
#include <QException>
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
}

int RoomService::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
   return m_rooms.length();
}

QVariant RoomService::data(const QModelIndex &index, int role) const
{
    if (role == NameRole)
    {
        return QVariant(m_rooms[index.row()].name());
    }

    if (role == TokenRole)
    {
        return QVariant(m_rooms[index.row()].token());
    }

    if (role == AccountRole)
    {
        return QVariant(m_rooms[index.row()].account().id());
    }

    if (role == UserIdRole)
    {
        return QVariant(m_rooms[index.row()].account().userId());
    }

    if (role == UnreadRole)
    {
        return QVariant(m_rooms[index.row()].unreadMessages());
    }

    if (role == MentionedRole)
    {
        return QVariant(m_rooms[index.row()].unreadMention());
    }

    return QVariant();
}

QHash<int, QByteArray> RoomService::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TokenRole] = "token";
    roles[AccountRole] = "accountId";
    roles[UserIdRole] = "accountUserId";
    roles[UnreadRole] = "unreadMessages";
    roles[MentionedRole] = "unreadMention";
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
        QString apiV = account->capabilities()->hasConversationV2() ? "v2" : "v1";
        endpoint.setPath(endpoint.path() + "/ocs/v2.php/apps/spreed/api/" + apiV + "/room");
        endpoint.setQuery("format=json");
        QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
        QNetworkReply* reply = m_nam.get(request);
        m_rooms_requests.append(reply);
    }
}

/**
 * @see https://github.com/nextcloud/spreed/blob/master/docs/constants.md
 */
bool RoomService::shallNotify(QJsonObject conversationData, Room oldConversationState) {
    int notificationLevel = conversationData.value("notificationLevel").toInt();

    if(notificationLevel == 3) { // never
        return false;
    }

    int lastMessageId = conversationData.value("lastMessage").toObject().value("id").toInt();
    int lastReadMessageId = std::max(
        conversationData.value("lastReadMessage").toInt(),
        m_db.lastKnownMessageId(oldConversationState.account().id(), conversationData.value("token").toString(), true)
    );
    if(lastReadMessageId >= lastMessageId) {
        return false;
    }

    int conversationType = conversationData.value("type").toInt();
    if(notificationLevel == 0) {
        // interpret default value
        notificationLevel = conversationType == 1 ? 1 : 2;
    }

    if(notificationLevel == 2) { // mentions
        bool wasMentioned = conversationData.value("unreadMention").toBool();
        if(wasMentioned && !oldConversationState.unreadMention()) {
            return true;
        }
    }

    if(notificationLevel == 1) { // always
        int unreadMessages = conversationData.value("unreadMessages").toInt();
        if(unreadMessages != oldConversationState.unreadMessages()) {
            return true;
        }
    }

    return false;
}

/**
 * @see https://github.com/nextcloud/spreed/blob/master/docs/constants.md
 */
bool RoomService::shallNotify(QJsonObject conversationData, int accountId) {
    int notificationLevel = conversationData.value("notificationLevel").toInt();

    if(notificationLevel == 3) { // never
        return false;
    }

    int lastMessageId = conversationData.value("lastMessage").toObject().value("id").toInt();
    int lastReadMessageId = std::max(
        conversationData.value("lastReadMessage").toInt(),
        m_db.lastKnownMessageId(accountId, conversationData.value("token").toString(), true)
    );
    if(lastReadMessageId >= lastMessageId) {
        // FIXME: or read state from server >= last message id. get it back to the other function, too
        return false;
    }

    int conversationType = conversationData.value("type").toInt();
    if(notificationLevel == 0) {
        // interpret default value
        notificationLevel = conversationType == 1 ? 1 : 2;
    }

    if(notificationLevel == 2) { // mentions
        return conversationData.value("unreadMention").toBool();
    }

    if(notificationLevel == 1) { // always
        return conversationData.value("unreadMessages").toInt() > 0;
    }

    return false;
}

QString RoomService::renderMessage(QString message, QJsonObject parameters, QString actorName) {
    QStringList keys = parameters.keys();
    foreach(const QString &key, keys) {
        QJsonObject parameter = parameters.value(key).toObject();
        message = message.replace('{' + key + '}', parameter.value("name").toString());
    }
    return actorName + ": " + message;
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
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    //qDebug() << "JSON" << payload;
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

    QJsonArray data = root.find("data").value().toArray();
    foreach(const QJsonValue& value, data) {
        QJsonObject room = value.toObject();
        Room model;
        model
                .setAccount(currentAccount)
                .setName(room.value("displayName").toString())
                .setFavorite(room.value("isFavorite").toBool())
                .setHasPassword(room.value("hasPassword").toBool())
                .setToken(room.value("token").toString())
                .setType(static_cast<Room::RoomType>(room.value("type").toInt()))
                .setUnreadMention(room.value("unreadMention").toBool())
                .setUnreadMessages(room.value("unreadMessages").toInt())
                .setLastActivity(room.value("lastActivity").toInt());

        try {
            Room knownRoom = findRoomByTokenAndAccount(model.token(), model.account().id());
            int i = m_rooms.indexOf(knownRoom);
            model.setNemoNotificationId(m_rooms.at(i).nemoNotificationId());
            m_rooms.replace(i, model);
            if(shallNotify(room, knownRoom)) {
                emitNotification(room, model, i);
            }
        } catch (QException& e) {
            Q_UNUSED(e)
            beginInsertRows(QModelIndex(), m_rooms.length(), m_rooms.length());
            m_rooms.append(model);
            endInsertRows();
            if(shallNotify(room, currentAccount->id())) {
                emitNotification(room, model, m_rooms.indexOf(model));
            }
        }
    }

    std::sort(m_rooms.begin(), m_rooms.end(), [](const Room& a, const Room b) {
        return a.lastActivity() > b.lastActivity();
    });
    dataChanged(index(0), index(m_rooms.length() - 1));
}

void RoomService::emitNotification(QJsonObject roomData, Room room, int index) {
    QJsonObject message = roomData.value("lastMessage").toObject();
    QString renderedMessage = renderMessage(
        message.value("message").toString(),
        message.value("messageParameters").toObject(),
        message.value("actorDisplayName").toString()
    );
    Notification notification;
    notification.setAppName("Nextcloud Talk");
    notification.setCategory("x-nextcloud.talk.im");
    notification.setSummary(roomData.value("displayName").toString());
    notification.setPreviewSummary(roomData.value("displayName").toString());
    notification.setMaxContentLines(3);
    notification.setBody(renderedMessage);
    notification.setPreviewBody(renderedMessage);
    if(m_rooms.at(index).nemoNotificationId() != 0) {
        notification.setReplacesId(m_rooms.at(index).nemoNotificationId());
    }

    QVariantList parameters;
    parameters.append(room.token());
    parameters.append(room.name());
    parameters.append(room.account().id());
    parameters.append(room.account().userId());

    QVariantList actions;
    actions.append(Notification::remoteAction(
        "default",
        "openConversation",
        "org.nextcloud.talk",
        "/org/nextcloud/talk",
        "org.nextcloud.talk",
        "openConversation",
        parameters
    ));
    notification.setRemoteActions(actions);

    //notification.publish();
    m_rooms[index].setNemoNotificationId(notification.replacesId());
}

Room RoomService::getRoom(QString token, int accountId) {
    QVector<Room>::iterator i;
    for(i = m_rooms.begin(); i != m_rooms.end(); i++) {
        if(i->token() == token && i->account().id() == accountId) {
            return *i;
        }
    }
    qDebug() << "No such room";
    QException e;
    throw e;
}

void RoomService::startPolling(QString token, int accountId) {
    activeToken = token;
    activeAccountId = accountId;
    m_isPolling = true;
    m_lookIntoFuture = 0;
    pollRoom();
}

bool RoomService::isPolling(QString token, int accountId) {
    return activeToken == token
            && activeAccountId == accountId
            && m_isPolling;
}

void RoomService::stopPolling() {
    m_isPolling = false;
}

Room RoomService::findRoomByTokenAndAccount(const QString token, const int accountId) {
    QVector<Room>::iterator i;
    for(i = m_rooms.begin(); i != m_rooms.end(); i++) {
        if(i->token() == token && i->account().id() == accountId) {
            return *i;
        }
    }
    QException e;
    throw e;
}

void RoomService::pollRoom() {
    if(!m_isPolling) {
        return;
    }
    NextcloudAccount* account;
    try {
        account = m_accountService->getAccountById(activeAccountId);
    } catch (QException &e) {
        Q_UNUSED(e)
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

void RoomService::sendMessage(QString messageText, int replyToId) {
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
    m_rooms.clear();
    endResetModel();
    loadRooms();
}
