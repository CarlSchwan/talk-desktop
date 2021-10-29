#pragma once

#include <QColor>
#include <QObject>
#include <QNetworkReply>
#include <QJsonObject>

class NextcloudAccount;

class Capabilities : public QObject
{
    Q_OBJECT

public:
    explicit Capabilities(NextcloudAccount *account);
    ~Capabilities() = default;
    bool areAvailable() const;
    void request();
    int getConversationApiLevel() const;
    QColor primaryColor() const;
    QUrl logoUrl() const;
    QString name() const;

public Q_SLOTS:
    void checkTalkCapHash(QNetworkReply *reply);

private:
    bool m_available = false;
    QJsonObject m_capabilities;
    QByteArray m_talkCapHash;
    NextcloudAccount *m_account;
};
