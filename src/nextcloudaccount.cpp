#include "services/capabilities.h"
#include "nextcloudaccount.h"
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

NextcloudAccount::NextcloudAccount(QObject *parent)
    : QObject(parent)
    , m_capabilities(new Capabilities(this))
    , m_qnam(new QNetworkAccessManager(this))
{
}

NextcloudAccount::NextcloudAccount(const int id, const QString &name, const QUrl &host,
        const QString &loginName, const QString &password, const QString &userId, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_host(host)
    , m_loginName(loginName)
    , m_password(password)
    , m_userId(userId)
    , m_capabilities(new Capabilities(this))
    , m_qnam(new QNetworkAccessManager(this))
{
}

NextcloudAccount::~NextcloudAccount()
{
}


NextcloudAccount *NextcloudAccount::fromSettings(const QSettings &settings, QObject *parent)
{
    auto account = new NextcloudAccount(parent);
    account->setId(settings.value("id").toInt());
    account->setName(settings.value("name").toString());
    account->setHost(settings.value("host").toUrl());
    account->setLoginName(settings.value("login_name").toString());
    account->setPassword(settings.value("password").toString());  // for compatibility/migration, dating back to alpha 7
    account->setUserId(settings.value("user_id").toString());

    if (settings.contains("colorOverride")) {
        account->setColorOverride(QColor(settings.value("colorOverride").toString()));
    }
    return account;
}

void NextcloudAccount::toSettings(QSettings &settings) const
{
    settings.setValue("id", m_id);
    settings.setValue("name", m_name);
    settings.setValue("host", m_host.url());
    settings.setValue("login_name", m_loginName);
    settings.setValue("password", "");
    settings.setValue("user_id", m_userId);
    if(m_colorOverride.isValid()) {
        settings.setValue("colorOverride", m_colorOverride);
    } else {
        settings.setValue("colorOverride", "");
    }
}

int NextcloudAccount::id() const
{
    return m_id;
}

void NextcloudAccount::setId(int id)
{
    if (id == m_id) {
        return;
    }
    m_id = id;
    m_dirty = true;
    Q_EMIT idChanged();
}

QString NextcloudAccount::name() const
{
    return m_name;
}

void NextcloudAccount::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    m_dirty = true;
    Q_EMIT nameChanged();
}

QUrl NextcloudAccount::host() const
{
    return m_host;
}

void NextcloudAccount::setHost(const QUrl &host) {
    if (m_host == host) {
        return;
    }
    m_host = host;
    m_dirty = true;
    Q_EMIT hostChanged();
}

QString NextcloudAccount::loginName() const
{
    return m_loginName;
}

void NextcloudAccount::setLoginName(const QString &loginName)
{
    if (m_loginName == loginName) {
        return;
    }
    m_loginName = loginName;
    m_dirty = true; 
    Q_EMIT loginNameChanged();
}

QString NextcloudAccount::password() const
{
    return m_password;
}

void NextcloudAccount::setPassword(const QString &password)
{
    if (m_password == password) {
        return;
    }
    m_password = password;
    m_dirty = true;
    Q_EMIT passwordChanged();
}

QString NextcloudAccount::userId() const
{
    return m_userId;
}

void NextcloudAccount::setUserId(const QString &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    m_dirty = true; 
    Q_EMIT userIdChanged();
}

QColor NextcloudAccount::colorOverride() const
{
    return m_colorOverride;
}

void NextcloudAccount::setColorOverride(const QColor &colorOverride)
{
    if (m_colorOverride == colorOverride) {
        return;
    }
    m_colorOverride = colorOverride;
    m_dirty = true;
    Q_EMIT colorOverrideChanged();
}

Capabilities *NextcloudAccount::capabilities() const
{
    return m_capabilities;
}

QNetworkRequest NextcloudAccount::setupRequest(const QUrl &url)
{
    QNetworkRequest request(url);

    const QString concatanated = m_loginName + QChar(':') + m_password;
    const QByteArray data = concatanated.toLocal8Bit().toBase64();
    const QString authValue = QLatin1String("Basic ") + data;

    request.setRawHeader(QByteArrayLiteral("Authorization"), authValue.toLocal8Bit());
#ifdef KDE_EDITION
    request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("Mozilla/5.0 Nextcloud Talk (Kirigami edition)"));
#else
    request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("Mozilla/5.0 Nextcloud Talk for SailfishOS/1.0"));
#endif

    if (url.path().indexOf("/ocs/") >= 0) {
        request.setRawHeader("OCS-APIRequest", "true");
    }
    return request;
}

void NextcloudAccount::get(const QUrl &url, std::function<void(QNetworkReply *)> callback)
{
    auto request = setupRequest(url);

    QNetworkReply *reply = m_qnam->get(request);

    if (callback != nullptr) {
        connect(reply, &QNetworkReply::finished, this, [reply, callback, url] () {
            if (200 != reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)) {
                qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << url;
                // TODO handle errors
                return;
            }

            callback(reply);
        });
    }
}

void NextcloudAccount::post(const QUrl &url, const QJsonDocument &doc, std::function<void(QNetworkReply *)> callback)
{
    const auto postData = doc.toJson();
    auto request = setupRequest(url);

    QNetworkReply *reply = m_qnam->post(request, postData);

    if (callback != nullptr) {
        connect(reply, &QNetworkReply::finished, this, [reply, callback, url, doc] () {
            if (200 != reply->attribute (QNetworkRequest::HttpStatusCodeAttribute)) {
                qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << url << doc.toJson();
                // TODO handle errors
                return;
            }

            callback(reply);
        });
    }
}

void NextcloudAccount::post(const QUrl &url, const QUrlQuery &formdata, std::function<void(QNetworkReply *)> callback)
{
    auto request = setupRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    const auto postData = formdata.toString().toLatin1();

    auto reply = m_qnam->post(request, postData);

    if (callback != nullptr) {
        connect(reply, &QNetworkReply::finished, this, [reply, callback, url] () {
            if (200 != reply->attribute (QNetworkRequest::HttpStatusCodeAttribute)) {
                qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << url;
                // TODO handle errors
                return;
            }

            callback(reply);
        });
    }
}
