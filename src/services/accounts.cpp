#include <qdebug.h>
#include <QObject>
#include <QSettings>
#include "accounts.h"
#include "capabilities.h"
#include "../db.h"

Accounts::Accounts(QObject *parent)
    : QAbstractListModel(parent)
{
    Q_UNUSED(parent)
}

Accounts* Accounts::getInstance()
{
    static Accounts* instance = new Accounts();
    return instance;
}

int Accounts::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return getInstance()->getAccounts().count() + 1;
}

QVariant Accounts::data(const QModelIndex &index, int role) const
{
    QVector<NextcloudAccount*> accounts = getInstance()->getAccounts();
    int knownAccounts = accounts.count();

    if (role == NameRole && index.row() == knownAccounts)
    {
        return QVariant("Add account");
    }
    else if (role == AccountRole && index.row() == knownAccounts)
    {
        return QVariant(-1);
    }

    if (role == NameRole)
    {
        return QVariant(accounts.at(index.row())->name());
    }
    else if (role == AccountRole && index.row() < knownAccounts)
    {
        return QVariant(accounts.at(index.row())->id());
    }

    if (role == LogoRole && index.row() < knownAccounts)
    {
        return QVariant(accounts.at(index.row())->capabilities()->logoUrl());
    } else if (role == LogoRole && index.row() == knownAccounts){
        return QVariant("image://theme/icon-m-add");
    }

    if (role == InstanceNameRole && index.row() < knownAccounts)
    {
        return QVariant(accounts.at(index.row())->capabilities()->name());
    } else if (role == LogoRole && index.row() == knownAccounts){
        return QVariant("");
    }

    if (role == ColorModeRole && index.row() < knownAccounts)
    {
        if (accounts.at(index.row())->colorOverride().isValid()) {
            return QVariant(Accounts::OverriddenColor);
        }
        return QVariant(Accounts::InstanceColor);
    } else if (role == LogoRole && index.row() == knownAccounts){
        return QVariant(Accounts::InstanceColor);
    }

    if (role == ColorRole && index.row() < knownAccounts)
    {
        if (accounts.at(index.row())->colorOverride().isValid()) {
            return QVariant(accounts.at(index.row())->colorOverride());
        }
        return QVariant(accounts.at(index.row())->capabilities()->primaryColor());
    } else if (role == LogoRole && index.row() == knownAccounts) {
        return QVariant(QColor("#006295"));
    }

    return QVariant();
}

bool Accounts::setData(const QModelIndex &index, const QVariant &value, int role) {
    QVector<NextcloudAccount*> accounts = getInstance()->getAccounts();

    if (role == ColorRole && index.row() < accounts.count())
    {
        accounts.at(index.row())->setColorOverride(value.value<QColor>());

        QSettings accountSettings("Nextcloud", "Accounts");
        accountSettings.beginGroup("account_" + QString::number(accounts.at(index.row())->id()));
        accounts.at(index.row())->toSettings(accountSettings);
        accountSettings.endGroup();
        accountSettings.sync();

        QVector<int> changedRole;
        changedRole.append(role);
        dataChanged(index, index, changedRole);
        return true;
    }

    return false;
}

QHash<int, QByteArray> Accounts::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AccountRole] = "account";
    roles[LogoRole] = "instanceLogo";
    roles[InstanceNameRole] = "instanceName";
    roles[ColorRole] = "color";
    roles[ColorModeRole] = "colorMode";
    return roles;
}

QString getSecretsKey(const NextcloudAccount* account) {
    return account->name() + " (" + QString::number(account->id()) +")";
}

void Accounts::addAccount(QString url, QString loginName, QString token, QString userId)
{
    loadAccounts();

    int id = ++max_id;
    QUrl host(url);
    QString name(loginName + "@" + host.host() + host.path());

    NextcloudAccount* account = new NextcloudAccount(id, name, host, loginName, token, userId);
    QSettings accountSettings("Nextcloud", "Accounts");

    beginResetModel();
    accountSettings.beginGroup("account_" + QString::number(id));
    account->toSettings(accountSettings);
    accountSettings.endGroup();
    accountSettings.sync();
    m_secrets.set(getSecretsKey(account), token.toUtf8());
    qDebug() << "account saved, status " << accountSettings.status();

    m_accounts.clear();
    endResetModel();
}

void Accounts::deleteAccount(int accountId)
{
    QSettings accountSettings("Nextcloud", "Accounts");
    NextcloudAccount* account = getAccountById(accountId);
    accountSettings.beginGroup("account_" + QString::number(accountId));
    accountSettings.remove("");
    accountSettings.endGroup();
    accountSettings.sync();
    m_secrets.unset(getSecretsKey(account));
    qDebug() << "account deleted, status " << accountSettings.status();

    Db db;
    db.deleteAccountEntries(accountId);

    NextcloudAccount* acc = getAccountById(accountId);
    if (!acc) {
        qDebug() << "Could not find account in vector for removal" << accountId;
        dataChanged(index(0), index(m_accounts.length() - 1));
        return;
    }

    int intIndex = m_accounts.indexOf(acc);
    beginRemoveRows(QModelIndex(), intIndex, intIndex);
    m_accounts.remove(intIndex);
    endRemoveRows();
}

NextcloudAccount* Accounts::getAccountById(const int id) {
    if(m_accounts.length() == 0) {
        readAccounts();
    }

    QVector<NextcloudAccount*>::iterator i;
    for(i = m_accounts.begin(); i != m_accounts.end(); i++) {
        if((*i)->id() == id) {
            return *i;
        }
    }
    return nullptr;
}

QVector<NextcloudAccount*> Accounts::getAccounts() {
    if(m_accounts.length() == 0) {
        readAccounts();
    }

    return m_accounts;
}

QVector<NextcloudAccount*> Accounts::readAccounts()
{
    qDebug() << "reading accs";
    QSettings accountSettings("Nextcloud", "Accounts");
    QStringList accountGroups = accountSettings.childGroups();

    if(m_accounts.count() > 0) {
        m_accounts.clear();
    }

    bool dirty = false;
    foreach(const QString &group, accountGroups) {
        accountSettings.beginGroup(group);
        NextcloudAccount* account = NextcloudAccount::fromSettings(accountSettings);
        if(account->password() != "") {
            // migration from pre-alpha7 where pwd was stored in plain text
            m_secrets.set(getSecretsKey(account), account->password().toUtf8());
            account->setPassword("");
            account->toSettings(accountSettings);
            dirty = true;
        }
        account->setPassword(m_secrets.get(getSecretsKey(account)));
        m_accounts.append(account);
        accountSettings.endGroup();
    }
    if(dirty) {
        accountSettings.sync();
    }
    qDebug() << "We have" << m_accounts.count() << "accounts";

    return m_accounts;
}

void Accounts::loadAccounts()
{
    beginResetModel();
    QVector<NextcloudAccount*> accounts = getInstance()->getAccounts();

    max_id = 0;
    foreach (NextcloudAccount* account, accounts) {
        if(account->id() > max_id) {
            max_id = account->id();
        }
    }

    qDebug() << "loading accs done, max id is" << max_id;
    endResetModel();
}
