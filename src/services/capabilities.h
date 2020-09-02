#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include "../nextcloudaccount.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class Capabilities : public QObject
{
    Q_OBJECT
public:
    Capabilities(NextcloudAccount *account);
    bool areAvailable() const;
    void request();
    NextcloudAccount *m_account;
    bool hasConversationV2() const;

private:
    QNetworkAccessManager m_nam;
    bool m_available = false;
    QJsonObject m_capabilities;
    QNetworkReply *m_reply = nullptr;

public slots:
    void requestFinished(QNetworkReply*);
    void handleError(QNetworkReply::NetworkError code) { qDebug() << "Error:" << code; }
};

#endif // CAPABILITIES_H
