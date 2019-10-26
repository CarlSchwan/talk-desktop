#ifndef DB_H
#define DB_H

#include <QObject>
#include <QSqlDatabase>

class Db : public QObject
{
    Q_OBJECT
public:
    Db();
    int lastKnownMessageId(int accountId, QString token);
    bool setLastKnownMessageId(int accountId, QString token, int messageId);

private:
    void initDb(QString dbPath);

    QSqlDatabase m_db;
};

#endif // DB_H
