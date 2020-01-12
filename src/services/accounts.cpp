#include <qdebug.h>
#include <QException>
#include <QObject>
#include <QSettings>
#include "accounts.h"
#include "../db.h"

Accounts::Accounts(QObject *parent)
    : QAbstractListModel(parent)
{
    Q_UNUSED(parent);
}

Accounts& Accounts::getInstance()
{
    static Accounts instance;
    return instance;
}

int Accounts::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return getInstance().getAccounts().count() + 1;
}

QVariant Accounts::data(const QModelIndex &index, int role) const
{
    QVector<NextcloudAccount> accounts = getInstance().getAccounts();
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
        return QVariant(accounts[index.row()].name());
    }
    else if (role == AccountRole && index.row() < knownAccounts)
    {
        return QVariant(accounts[index.row()].id());
    }

    return QVariant();
}

QHash<int, QByteArray> Accounts::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AccountRole] = "account";
    return roles;
}

void Accounts::addAccount(QString url, QString loginName, QString token, QString userId)
{
    loadAccounts();

    int id = ++max_id;
    QUrl host(url);
    QString name(loginName + "@" + host.host() + host.path());

    NextcloudAccount account(id, name, host, loginName, token, userId);
    QSettings accountSettings("Nextcloud", "Accounts");

    accountSettings.beginGroup("account_" + QString::number(id));
    account.toSettings(accountSettings);
    accountSettings.endGroup();
    accountSettings.sync();
    qDebug() << "account saved, status " << accountSettings.status();

    m_accounts.clear();
    readAccounts();
    loadAccounts();
}

void Accounts::deleteAccount(int accountId)
{
    QSettings accountSettings("Nextcloud", "Accounts");
    accountSettings.beginGroup("account_" + QString::number(accountId));
    accountSettings.remove("");
    accountSettings.endGroup();
    accountSettings.sync();
    qDebug() << "account deleted, status " << accountSettings.status();

    Db db;
    db.deleteAccountEntries(accountId);

    NextcloudAccount acc;
    try {
        acc = getAccountById(accountId);
    } catch (QException &e) {
        qDebug() << "Could not find account in vector for removal" << accountId;
        dataChanged(index(0), index(m_accounts.length() - 1));
        return;
    }

    int intIndex = m_accounts.indexOf(acc);
    beginRemoveRows(QModelIndex(), intIndex, intIndex);
    //readAccounts();
    m_accounts.remove(intIndex);
    //m_accounts.erase(i);
    loadAccounts();
    beginRemoveRows(QModelIndex(), intIndex, intIndex);
}

NextcloudAccount Accounts::getAccountById(const int id) {
    if(m_accounts.length() == 0) {
        readAccounts();
    }

    QVector<NextcloudAccount>::iterator i;
    for(i = m_accounts.begin(); i != m_accounts.end(); i++) {
        qDebug() << "testing" << i->id() << "with" << id;
        if(i->id() == id) {
            return *i;
        }
    }
    qDebug() << "No such account";
    QException e;
    throw e;
}

QVector<NextcloudAccount> Accounts::getAccounts() {
    if(m_accounts.length() == 0) {
        readAccounts();
    }

    return m_accounts;
}

QVector<NextcloudAccount> Accounts::readAccounts()
{
    qDebug() << "reading accs";
    QSettings accountSettings("Nextcloud", "Accounts");
    QStringList accountGroups = accountSettings.childGroups();

    if(m_accounts.count() > 0) {
        m_accounts.clear();
    }

    foreach(const QString &group, accountGroups) {
        accountSettings.beginGroup(group);
        m_accounts.append(NextcloudAccount::fromSettings(accountSettings));
        accountSettings.endGroup();
    }
    qDebug() << "We have" << m_accounts.count() << "accounts";

    return m_accounts;
}

void Accounts::loadAccounts()
{
    beginResetModel();
    QVector<NextcloudAccount> accounts = getInstance().getAccounts();

    max_id = 0;
    foreach (NextcloudAccount account, accounts) {
        if(account.id() > max_id) {
            max_id = account.id();
        }
    }

    qDebug() << "loading accs done, max id is" << max_id;
    endResetModel();
}
