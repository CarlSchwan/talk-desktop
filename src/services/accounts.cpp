#include <qdebug.h>
#include <QObject>
#include <QSettings>
#include "accounts.h"
#include "capabilities.h"
#include "../db.h"

AccountModel::AccountModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

AccountModel* AccountModel::getInstance()
{
    static AccountModel* instance = new AccountModel();
    return instance;
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_accounts.count();
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    const int knownAccounts = m_accounts.count();

    auto account = m_accounts.at(index.row());
    switch (role) {
    case NameRole:
        return account->name();
    case LogoRole:
        return account->capabilities()->logoUrl();
    case InstanceNameRole:
        return account->capabilities()->name();
    case ColorModeRole:
        if (account->colorOverride().isValid()) {
            return AccountModel::OverriddenColor;
        }
        return AccountModel::InstanceColor;
    case ColorRole:
        if (account->colorOverride().isValid()) {
            return account->capabilities()->primaryColor();
        } else {
            return QColor("#006295");
        }
    }

    return {};
}

bool AccountModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == ColorRole && index.isValid()) {
        m_accounts.at(index.row())->setColorOverride(value.value<QColor>());

        QSettings accountSettings(QStringLiteral("Nextcloud"), QStringLiteral("Accounts"));
        accountSettings.beginGroup(QStringLiteral("account_") + QString::number(m_accounts.at(index.row())->id()));
        m_accounts.at(index.row())->toSettings(accountSettings);
        accountSettings.endGroup();
        accountSettings.sync();

        QVector<int> changedRole;
        changedRole.append(role);
        dataChanged(index, index, changedRole);
        return true;
    }

    return false;
}

QHash<int, QByteArray> AccountModel::roleNames() const {
    return {
        {NameRole, QByteArrayLiteral("name")},
        {AccountRole, QByteArrayLiteral("account")},
        {LogoRole, QByteArrayLiteral("instanceLogo")},
        {InstanceNameRole, QByteArrayLiteral("instanceName")},
        {ColorRole, QByteArrayLiteral("color")},
        {ColorModeRole, QByteArrayLiteral("colorMode")}
    };
}

QString getSecretsKey(const NextcloudAccount *account) {
    return account->name() + " (" + QString::number(account->id()) +")";
}

void AccountModel::addAccount(const QString &url, const QString &loginName, const QString &token, const QString &userId)
{
    qDebug() << "add" << url << loginName << token << userId;
    const int id = ++m_maxId;
    const QUrl host(url);
    const QString name(loginName + QChar('@') + host.host() + host.path());

    auto account = new NextcloudAccount(name, host, loginName, token, userId, this);
    QSettings accountSettings(QStringLiteral("Nextcloud"), QStringLiteral("Accounts"));

    accountSettings.beginGroup(QStringLiteral("account_") + QString::number(id));
    account->toSettings(accountSettings);
    accountSettings.endGroup();
    accountSettings.sync();
    m_secrets.set(getSecretsKey(account), token.toUtf8());

    beginInsertRows({}, m_accounts.count(), m_accounts.count());
    m_accounts.append(account);
    endInsertRows();
}

void AccountModel::deleteAccount(int accountId)
{
    QSettings accountSettings("Nextcloud", "Accounts");
    auto account = getAccountById(accountId);
    accountSettings.beginGroup("account_" + QString::number(accountId));
    accountSettings.remove("");
    accountSettings.endGroup();
    accountSettings.sync();
    m_secrets.unset(getSecretsKey(account));
    qDebug() << "account deleted, status " << accountSettings.status();

    Db db;
    db.deleteAccountEntries(account);

    NextcloudAccount* acc = getAccountById(accountId);
    if (!acc) {
        qDebug() << "Could not find account in vector for removal" << accountId;
        dataChanged(index(0), index(m_accounts.length() - 1));
        return;
    }

    int intIndex = m_accounts.indexOf(acc);
    beginRemoveRows(QModelIndex(), intIndex, intIndex);
    acc->deleteLater();
    m_accounts.remove(intIndex);
    endRemoveRows();
}

QVector<NextcloudAccount*> AccountModel::getAccounts() const
{
    return m_accounts;
}

void AccountModel::load()
{
    QSettings accountSettings("Nextcloud", "Accounts");
    const QStringList accountGroups = accountSettings.childGroups();

    bool dirty = false;
    for (const QString &group : accountGroups) {
        accountSettings.beginGroup(group);
        auto account = NextcloudAccount::fromSettings(accountSettings, this);
        if (account->password() != "") {
            // migration from pre-alpha7 where pwd was stored in plain text
            m_secrets.set(getSecretsKey(account), account->password().toUtf8());
            account->setPassword("");
            account->toSettings(accountSettings);
            dirty = true;
        }
        account->setPassword(m_secrets.get(getSecretsKey(account)));
        beginInsertRows({}, m_accounts.count(), m_accounts.count());
        m_accounts.append(account);
        if (account->password() == "") {
            deleteAccount(getAccountId(account));
            continue;
        }
        if(account->id() > m_maxId) {
            m_maxId = account->id();
        }
        endInsertRows();
        accountSettings.endGroup();
    }

    if (dirty) {
        accountSettings.sync();
    }
}

int AccountModel::getAccountId(NextcloudAccount *account) const
{
    for (int i = 0; i < m_accounts.count(); i++) {
        if (m_accounts[i] == account) {
            return i;
        }
    }
    return -1;
}

NextcloudAccount *AccountModel::getAccountById(int accountId) const
{
    return m_accounts[accountId];
}
