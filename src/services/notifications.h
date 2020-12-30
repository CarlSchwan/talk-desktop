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

private:
    void processPayload(QNetworkReply* reply);
    void processNotificationData(const QJsonObject data, const int accountId);
    static const QString NC_NOTIFICATION_ENDPOINT;
    QVector<NextcloudAccount*> m_accounts;
    int m_notificationStateId = 0;
    QNetworkAccessManager m_nam;
    //QVector<Notification> m_notifications;
    QMap<qint64, QSharedPointer<Notification>> m_notifications;
    Accounts* m_accountService;
    QTimer* m_pollTimer = new QTimer(this);
};
#endif // NOTIFICATIONS_H
