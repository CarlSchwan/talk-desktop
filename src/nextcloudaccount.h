#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QUrl>
#include <QSettings>
#include <QSharedPointer>
#include <QNetworkReply>

class QNetworkReply;
class QNetworkAccessManager;
class Capabilities;

class NextcloudAccount : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QUrl host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(QString loginName READ loginName WRITE setLoginName NOTIFY loginNameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString userId READ userId WRITE setUserId NOTIFY userIdChanged)

    NextcloudAccount(QObject *parent = nullptr);
public:
    NextcloudAccount(
        const int id,
        const QString &name,
        const QUrl &host,
        const QString &loginName,
        const QString &password,
        const QString &userId,
        QObject *parent = nullptr
    );

    ~NextcloudAccount();

    static NextcloudAccount *fromSettings(const QSettings &settings, QObject *parent = nullptr);
    void toSettings(QSettings &settings) const;

    // Base methods to interact with the Nextcloud server.
    QNetworkRequest setupRequest(const QUrl &url);
    void get(const QUrl &url, std::function<void(QNetworkReply *)> callback);
    void post(const QUrl &url, const QJsonDocument &doc, std::function<void(QNetworkReply *)> callback);
    void post(const QUrl &url, const QUrlQuery &formdata, std::function<void(QNetworkReply *)> callback);

    int id() const;
    QString name() const;
    QUrl host() const;
    QString loginName() const;
    QString password() const;
    QString userId() const;
    QColor colorOverride() const;
    Capabilities *capabilities() const;

    void setId(int id);
    void setName(const QString &name);
    void setHost(const QUrl &host);
    void setLoginName(const QString &loginName);
    void setPassword(const QString &password);
    void setUserId(const QString &userId);
    void setColorOverride(const QColor &colorOverride);

Q_SIGNALS:
    void idChanged();
    void nameChanged();
    void hostChanged();
    void loginNameChanged();
    void passwordChanged();
    void userIdChanged();
    void colorOverrideChanged();

private:
    int m_id;
    QString m_name;
    QUrl m_host;
    QString m_loginName;
    QString m_password;
    QString m_userId;
    bool m_dirty = false;
    QColor m_colorOverride;
    Capabilities *m_capabilities = nullptr;
    QNetworkAccessManager *m_qnam = nullptr;


};
