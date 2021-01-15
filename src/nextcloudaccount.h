#ifndef NEXTCLOUDACCOUNT_H
#define NEXTCLOUDACCOUNT_H

#include <QString>
#include <QUrl>
#include <QSettings>
#include <QSharedPointer>

QT_FORWARD_DECLARE_CLASS(Capabilities)

class NextcloudAccount
{
    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QUrl host READ host WRITE setHost)
    Q_PROPERTY(QString login_name READ loginName WRITE setLoginName)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString user_id READ userId WRITE setUserId)
public:
    NextcloudAccount();
    NextcloudAccount(
        const int id,
        const QString name,
        const QUrl host,
        const QString login_name,
        const QString password,
        const QString user_id
    );
    NextcloudAccount(const NextcloudAccount& account);
    ~NextcloudAccount();
    static NextcloudAccount* fromSettings(const QSettings &settings);
    void toSettings(QSettings &settings) const;
    bool operator ==(const NextcloudAccount &toCompare) const;

private:
    int m_id;
    QString m_name;
    QUrl m_host;
    QString m_login_name;
    QString m_password;
    QString m_user_id;
    bool m_dirty = false;
    QSharedPointer<Capabilities> m_capabilities;

public slots:
    int id() const;
    QString name() const;
    QUrl host() const;
    QString loginName() const;
    QString password() const;
    QString userId() const;
    Capabilities *capabilities() const;

    void setId(const int id);
    void setName(const QString name);
    void setHost(const QUrl host);
    void setLoginName(const QString login_name);
    void setPassword(const QString password);
    void setUserId(const QString user_id);
};

#endif // NEXTCLOUDACCOUNT_H
