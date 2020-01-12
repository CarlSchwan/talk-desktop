#include "nextcloudaccount.h"

NextcloudAccount::NextcloudAccount() {}

NextcloudAccount::NextcloudAccount(
    const int id,
    const QString name,
    const QUrl host,
    const QString login_name,
    const QString password,
    const QString user_id
)
{
    m_id = id;
    m_name = name;
    m_host = host;
    m_login_name = login_name;
    m_password = password;
    m_user_id = user_id;
}

NextcloudAccount::NextcloudAccount(const NextcloudAccount& account)
{
    m_id = account.id();
    m_name = account.name();
    m_host = account.host();
    m_login_name = account.loginName();
    m_password = account.password();
    m_user_id = account.userId();
}

NextcloudAccount NextcloudAccount::fromSettings(const QSettings &settings)
{
    return NextcloudAccount(
        settings.value("id").toInt(),
        settings.value("name").toString(),
        settings.value("host").toUrl(),
        settings.value("login_name").toString(),
        settings.value("password").toString(),
        settings.value("user_id").toString()
    );
}

void NextcloudAccount::toSettings(QSettings &settings) const
{
    settings.setValue("id", m_id);
    settings.setValue("name", m_name);
    settings.setValue("host", m_host.url());
    settings.setValue("login_name", m_login_name);
    settings.setValue("password", m_password);
    settings.setValue("user_id", m_user_id);
}

int NextcloudAccount::id() const { return m_id; }
void NextcloudAccount::setId(const int id) { m_id = id; m_dirty = true; }

QString NextcloudAccount::name() const { return m_name; }
void NextcloudAccount::setName(const QString name) { m_name = name; m_dirty = true; }

QUrl NextcloudAccount::host() const { return m_host; }
void NextcloudAccount::setHost(const QUrl host) { m_host = host; m_dirty = true; }

QString NextcloudAccount::loginName() const { return m_login_name; }
void NextcloudAccount::setLoginName(const QString login_name) { m_login_name = login_name; m_dirty = true; }

QString NextcloudAccount::password() const { return m_password; }
void NextcloudAccount::setPassword(const QString password) { m_password = password; m_dirty = true; }

QString NextcloudAccount::userId() const { return m_user_id; }
void NextcloudAccount::setUserId(const QString user_id) { m_user_id = user_id; m_dirty = true; }

bool NextcloudAccount::operator ==(const NextcloudAccount &toCompare) const {
    return toCompare.id() == id();
}
