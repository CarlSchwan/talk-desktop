#include "accountmodel.h"
#include "QtCore/qdebug.h"

#include <QSettings>

AccountModel::AccountModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    return accounts.count() + 1;
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    if (role == NameRole && index.row() == accounts.count())
    {
        return QVariant("Add account");
    }
    else if (role == AccountRole && index.row() == accounts.count())
    {
        return QVariant(-1);
    }

    if (role == NameRole)
    {
        return QVariant(accounts[index.row()].name());
    }
    else if (role == AccountRole && index.row() < accounts.count())
    {
        return QVariant(accounts[index.row()].id());
    }

    return QVariant();
}

void AccountModel::loadAccounts()
{
    QSettings accountSettings("Nextcloud", "Accounts");
    QStringList accountGroups = accountSettings.childGroups();
    beginResetModel();
    accounts.clear();

    foreach(const QString &group, accountGroups) {
        accountSettings.beginGroup(group);
        accounts.append(NextcloudAccount::fromSettings(accountSettings));
        if(accountSettings.value("id").toInt() > max_id) {
            max_id = accountSettings.value("id").toInt();
        }
        accountSettings.endGroup();
    }
    qDebug() << "loading accs done, max id is" << max_id;
    endResetModel();
}

QHash<int, QByteArray> AccountModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AccountRole] = "account";
    return roles;
}

void AccountModel::addAccount(QString url, QString loginName, QString token, QString userId)
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
}
