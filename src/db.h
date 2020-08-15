#ifndef DB_H
#define DB_H

#include <QObject>
#include <QSqlDatabase>

class Db : public QObject
{
    Q_OBJECT
public:
    Db();
    int lastKnownMessageId(int accountId, QString token, bool silent = false);
    bool setLastKnownMessageId(int accountId, QString token, int messageId);
    bool deleteAccountEntries(int accountId);

private:
    void initDb(QString dbPath);

    QSqlDatabase m_db;
};

#endif // DB_H
