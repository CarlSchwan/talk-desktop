#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QNetworkReply>
#include <QTimer>
#include <QVector>
#include <nemonotifications-qt5/notification.h>
#include "../nextcloudaccount.h"
#include "accounts.h"

class Notifications : public QObject
{
    Q_OBJECT
public:
    Notifications();
    ~Notifications();

public slots:
    void watchAccounts(QVector<NextcloudAccount*> accounts);
    void watchAccounts(Accounts* accountService);

private slots:
    void notificationPayloadReceived(QNetworkReply* reply);
    void watchAccounts();
    void afterCloseNotification(int ncNotificationId, int accountId);
    void afterActiveConversationChanged(QString tolen, int accountId);

private:
    void processPayload(QNetworkReply* reply);
    void processNotificationData(const QJsonObject data, const int accountId);
    void removeNotificationsExternallyDismissed(const int accountId, const int pullCycleId);
    static const QString NC_NOTIFICATION_ENDPOINT;
    static const int STARTUP_INTERVAL;
    static const int REGULAR_INTERVAL;
    QVector<NextcloudAccount*> m_accounts;
    // ID of notification pull cycle
    int m_notificationStateId = 0;
    QNetworkAccessManager m_nam;
    QMap<qint64, QSharedPointer<Notification>> m_notifications;
    Accounts* m_accountService;
    QTimer* m_pollTimer = new QTimer(this);
};
#endif // NOTIFICATIONS_H
