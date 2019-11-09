#include "db.h"

#include <QDir>
#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

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

int Db::lastKnownMessageId(int accountId, QString token) {
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT messageId FROM lastKnownMessageMarkers "
        "WHERE accountId=:accountId "
            "AND token=:token"
    );
    query.bindValue(":accountId", accountId);
    query.bindValue(":token", token);
    query.exec();
    if(!query.first()) {
        qDebug() << query.boundValues();
        qDebug() << query.lastError();
        return 0;
    }
    int id = query.value(0).toInt();
    return id;
}

bool Db::setLastKnownMessageId(int accountId, QString token, int messageId) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO lastKnownMessageMarkers "
        "VALUES(:accountId, :token, :messageId);"
    );
    query.bindValue(":accountId", accountId);
    query.bindValue(":token", token);
    query.bindValue(":messageId", messageId);
    bool result = query.exec();
    if(!result) {
        qCritical() << "Failed to save last known message id "
                    << "Query " << query.boundValues()
                    << " :" << m_db.lastError().text();
    }
    return result;
}

void Db::initDb(QString dbPath) {
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
