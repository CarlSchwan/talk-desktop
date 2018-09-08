#include "accountmodel.h"
#include "QtCore/qdebug.h"

#include <QSettings>

AccountModel::AccountModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadAccounts();
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    return accounts.count() + 1;
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    if(!is_initialized) {
        //loadAccounts(); not const, need to initalize differently
    }

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

    foreach(const QString &group, accountGroups) {
        accountSettings.beginGroup(group);
        accounts.append(NextcloudAccount::fromSettings(accountSettings));
        accountSettings.endGroup();
    }
}

QHash<int, QByteArray> AccountModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AccountRole] = "account";
    return roles;
}
