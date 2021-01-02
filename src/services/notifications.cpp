#include <QDBusConnection>
#include <QDBusError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVectorIterator>
#include "notifications.h"
#include "requestfactory.h"

const QString Notifications::NC_NOTIFICATION_ENDPOINT = "/ocs/v2.php/apps/notifications/api/v2/notifications";
const int Notifications::STARTUP_INTERVAL = 4000;
const int Notifications::REGULAR_INTERVAL = 29000;

Notifications::Notifications(): QObject ()
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &Notifications::notificationPayloadReceived);
    connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(watchAccounts()));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.connect("", "/conversation", "org.nextcloud.talk", "afterActiveConversationChanged",
                this, SLOT(afterActiveConversationChanged(QString, int)));
}

Notifications::~Notifications()
{
    foreach(QSharedPointer<Notification> n, m_notifications)
    {
        n->close();
    }
}

void Notifications::watchAccounts(QVector<NextcloudAccount*> accounts)
{
    m_notificationStateId++;
    m_accounts = accounts;
    RequestFactory rf;
    foreach(NextcloudAccount* account, m_accounts)
    {
        QUrl endpoint = QUrl(account->host());
        endpoint.setPath(endpoint.path() + NC_NOTIFICATION_ENDPOINT);
        endpoint.setQuery("format=json");
        QNetworkRequest request = rf.getRequest(endpoint, account);
        QNetworkReply* reply = m_nam.get(request);
        reply->setProperty("AccountID", account->id());
        reply->setProperty("NotificationStateId", m_notificationStateId);
    }
}

void Notifications::watchAccounts(Accounts* accountService)
{
    m_accountService = accountService;
    m_pollTimer->start(4000);
}

void Notifications::watchAccounts()
{
    if(m_pollTimer->interval() == STARTUP_INTERVAL)
    {
        m_pollTimer->setInterval(REGULAR_INTERVAL);
    }
    watchAccounts(m_accountService->getAccounts());
}

void Notifications::notificationPayloadReceived(QNetworkReply* reply)
{
    QVariant aid = reply->property("AccountID");
    QVariant nsid = reply->property("NotificationStateId");
    if(
        !aid.isValid()
        || !nsid.isValid()
        || nsid.toInt() != m_notificationStateId  // ignore replies for old requests
    ) {
        return;
    }

    processPayload(reply);
}

void Notifications::processPayload(QNetworkReply* reply)
{
    QString userStatus = reply->rawHeader("X-Nextcloud-User-Status");
    if(userStatus == "dnd") {
        // »in case of dnd no notifications should be directly shown«
        // https://github.com/nextcloud/notifications/blob/master/docs/ocs-endpoint-v2.md
        return;
    }

    QByteArray payload = reply->readAll();
    QJsonDocument apiResult = QJsonDocument::fromJson(payload);
    QJsonObject q = apiResult.object();
    QJsonObject root = q.find("ocs").value().toObject();
    //qDebug() << "Participant JSON" << payload;

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

    QJsonArray data = root.constFind("data").value().toArray();
    const int accountId = reply->property("AccountID").toInt();
    foreach(QJsonValue v, data) {
        processNotificationData(v.toObject(), accountId);
    }
    removeNotificationsExternallyDismissed(accountId, reply->property("NotificationStateId").toInt());
}

void Notifications::processNotificationData(const QJsonObject data, const int accountId)
{
    if(data.find("app").value().toString() != "spreed") {
        return;
    }

    if(data.find("object_type").value().toString() != "chat") {
        return;
    }

    QJsonObject subjectParameters = data.find("subjectRichParameters").value().toObject();
    if(!subjectParameters.contains("call")) {
        qDebug() << "Missing room name" << data;
        return;
    }

    int ncNotificationId = data.constFind("notification_id").value().toInt();
    if(m_notifications.contains(ncNotificationId)) {
        return;
    }

    QString roomName = subjectParameters.constFind("call").value().toObject().find("name").value().toString();
    QString roomId = data.constFind("object_id").value().toString();
    QString userId = subjectParameters.constFind("user").value().toObject().find("id").value().toString();
    QString userName = subjectParameters.constFind("user").value().toObject().find("name").value().toString();
    auto dateTime = QDateTime::fromString(data.constFind("datetime").value().toString(), Qt::ISODate);
    QString renderedMessage = data.constFind("message").value().toString();
    QString renderedSubject = data.constFind("subject").value().toString();

    QSharedPointer<Notification> notification = QSharedPointer<Notification>(new Notification);
    notification->setAppName("Nextcloud Talk");
    notification->setCategory("x-nextcloud.talk.im");
    notification->setSummary(roomName);
    notification->setPreviewSummary(roomName);
    notification->setMaxContentLines(4);
    notification->setBody(renderedSubject + ": " + renderedMessage);
    notification->setPreviewBody(renderedMessage);
    notification->setTimestamp(dateTime);
    notification->setProperty("NcNotificationId", ncNotificationId);
    notification->setProperty("NcRoomId", roomId);
    notification->setProperty("NcAccountId", accountId);
    notification->setProperty("PullCycleId", m_notificationStateId); // not necessarily the original cycle

    QVariantList parameters;
    parameters.append(roomId);
    parameters.append(roomName);
    parameters.append(accountId);
    parameters.append(userId);

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
    notification->setRemoteActions(actions);

    connect(&*notification, &Notification::closed,
            [=]( const uint &reason ) {
                Q_UNUSED(reason)
                this->afterCloseNotification(ncNotificationId, accountId);
            }
    );

    notification->publish();
    m_notifications[ncNotificationId] = notification;
}

void Notifications::afterCloseNotification(int ncNotificationId, int accountId)
{
    RequestFactory rf;
    NextcloudAccount* account = m_accountService->getAccountById(accountId);
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + NC_NOTIFICATION_ENDPOINT + "/" + QString::number(ncNotificationId));
    QNetworkRequest request = rf.getRequest(endpoint, account);
    m_nam.deleteResource(request);
}

void Notifications::afterActiveConversationChanged(QString token, int accountId)
{
    foreach(QSharedPointer<Notification> n, m_notifications)
    {
        if(n->property("NcRoomId") == token && n->property("NcAccountId") == accountId) {
            // notifications are dismissed by Nextcloud automatically on entering a conversation
            n->close();
        }
    }
}

void Notifications::removeNotificationsExternallyDismissed(const int accountId, const int pullCycleId)
{
    foreach(QSharedPointer<Notification> n, m_notifications)
    {
        if(n->property("NcAccountId") == accountId && n->property("PullCycleId").toInt() < pullCycleId) {
            // notifications was dismissed externally
            n->close();
        }
    }
}
