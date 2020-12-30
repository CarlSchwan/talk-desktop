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
    m_pollTimer->setInterval(29000);
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
    // FIXME: remove m_notificationStateId (or remember what it was meant to be for)
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
        qDebug() << "Notification watching" << account->id();
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
    qDebug() << "Notification payload receided aid " << reply->property("AccountID") << "nsid" << reply->property("NotificationStateId");

    QVariant aid = reply->property("AccountID");
    QVariant nsid = reply->property("NotificationStateId");
    if(
        !aid.isValid()
        || !nsid.isValid()
        || nsid.toInt() != m_notificationStateId
    ) {
        return;
    }

    processPayload(reply);
    if(!m_pollTimer->isActive()) {
        m_pollTimer->start();
    }
}

void Notifications::processPayload(QNetworkReply* reply)
{
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

    QJsonArray data = root.constFind("data").value().toArray();
    foreach(QJsonValue v, data) {
        processNotificationData(v.toObject(), reply->property("AccountID").toInt());
    }

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

    qDebug() << "Checking for present notifications";
    qDebug() << m_notifications;
    int ncNotificationId = data.constFind("notification_id").value().toInt();
    if(m_notifications.contains(ncNotificationId)) {
        return;
    }

    QString roomName = subjectParameters.constFind("call").value().toString();
    QString roomId = data.constFind("object_id").value().toString();
    QString userId = data.constFind("user").value().toString();
    auto dateTime = QDateTime::fromString(data.constFind("datetime").value().toString(), Qt::ISODate);
    QString renderedMessage = data.constFind("subject").value().toString();

    QSharedPointer<Notification> notification = QSharedPointer<Notification>(new Notification);
    notification->setAppName("Nextcloud Talk");
    notification->setCategory("x-nextcloud.talk.im");
    notification->setSummary(roomName);
    notification->setPreviewSummary(roomName);
    notification->setMaxContentLines(3);
    notification->setBody(renderedMessage);
    notification->setPreviewBody(renderedMessage);
    notification->setTimestamp(dateTime);
    notification->setProperty("NcNotificationId", ncNotificationId);
    notification->setProperty("NcRoomId", roomId);
    notification->setProperty("NcAccountId", accountId);

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
