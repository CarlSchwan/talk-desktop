#include "accountreader.h"

AccountReader::AccountReader()
{

}

QVector<NextcloudAccount> AccountReader::readAccounts()
{
    QSettings accountSettings("Nextcloud", "Accounts");
    QStringList accountGroups = accountSettings.childGroups();
    QVector<NextcloudAccount> accounts;

    foreach(const QString &group, accountGroups) {
        accountSettings.beginGroup(group);
        accounts.append(NextcloudAccount::fromSettings(accountSettings));
        accountSettings.endGroup();
    }

    return accounts;
}
