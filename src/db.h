#pragma once

#include <QObject>
#include <QSqlDatabase>

class NextcloudAccount;

class Db
{
public:
    explicit Db();

    int lastKnownMessageId(NextcloudAccount *account, const QString &roomToken, bool silent = false);
    bool setLastKnownMessageId(NextcloudAccount *account, const QString &roomToken, int messageId);
    bool deleteAccountEntries(NextcloudAccount *account);

private:
    void initDb(const QString &dbPath);

    QSqlDatabase m_db;
};
