#include "db.h"

#include <QDir>
#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include "nextcloudaccount.h"

Db::Db()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if(dataDir.length() && dataDir.endsWith("/harbour-nextcloud-talk/harbour-nextcloud-talk")) {
        dataDir = dataDir.split("/harbour-nextcloud-talk/harbour-nextcloud-talk").at(0);
    }
    dataDir += "/harbour-nextcloud-talk";
    QString dbPath = dataDir + "/talk.db";

    if(!QDir(dataDir).exists()) {
        QDir().mkpath(dataDir);
        qDebug() << "dir created " << dataDir;
    }

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    if (!m_db.isValid()) {
        qCritical() << "QSQLITE database driver is not available.";
        return;
    }

    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qCritical() << "Cannot open talk database"
                    << dbPath
                    << ":" << m_db.lastError().text();
        return;
    }
    initDb(dbPath);
}

int Db::lastKnownMessageId(NextcloudAccount *account, const QString &token, bool silent) {
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT messageId FROM lastKnownMessageMarkers "
        "WHERE accountId=:accountId "
            "AND token=:token"
    );
    query.bindValue(":accountId", account->host().toString() + account->userId());
    query.bindValue(":token", token);
    query.exec();
    if(!query.first()) {
        if(!silent) {
            qDebug() << query.boundValues();
            qDebug() << query.lastError();
        }
        return 0;
    }
    return query.value(0).toInt();
}

bool Db::setLastKnownMessageId(NextcloudAccount *account, const QString &token, int messageId) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO lastKnownMessageMarkers "
        "VALUES(:accountId, :token, :messageId);"
    );
    query.bindValue(":accountId", account->host().toString() + account->userId());
    query.bindValue(":token", token);
    query.bindValue(":messageId", messageId);
    const bool result = query.exec();
    if(!result) {
        qCritical() << "Failed to save last known message id "
                    << "Query " << query.boundValues()
                    << " :" << m_db.lastError().text();
    }
    return result;
}

bool Db::deleteAccountEntries(NextcloudAccount *account)
{
    QSqlQuery query;
    query.prepare("DELETE FROM lastKnownMessageMarkers WHERE accountId = :accountId;");
    query.bindValue(":accountId", account->host().toString() + account->userId());
    const bool result = query.exec();
    if(!result) {
        qWarning() << "Failed to delete entries for of an account"
                    << "Query " << query.boundValues()
                    << " :" << m_db.lastError().text();
    }
    return result;
}

void Db::initDb(const QString &dbPath) {
    if(!m_db.transaction()) {
        qCritical() << "Cannot start transaction"
                    << dbPath
                    << ":" << m_db.lastError().text();
        return;
    }
    QSqlQuery query;
    query.exec(
        "CREATE TABLE IF NOT EXISTS lastKnownMessageMarkers "
        "(accountId INTEGER, token TEXT, messageId UNSIGNED BIG INT, "
        "PRIMARY KEY(accountId, token))"
    );
    query.exec("PRAGMA user_version=3");
    if(!m_db.commit()) {
        qCritical() << "Finishing transaction failed"
                    << dbPath
                    << ":" << m_db.lastError().text();
        m_db.rollback();
    }
}
