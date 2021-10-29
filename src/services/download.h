#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkReply>
#include "accounts.h"

class Download : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Download)
    Download() {}
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        return new Download;
    }

public slots:
    void getFile(const QString &path, int accountId);
    bool fileExists(const QString &path, int accountId);
    QString filePath(const QString &path, int accountId);

signals:
    void fileDownloaded(const QString &localFile);

private slots:
    void downloadFinished(QNetworkReply *reply);

private:
    class NcDownload
    {
    public:
        NcDownload() {}
        NcDownload(const QString &target, QNetworkReply *nwReply)
            : m_target(target)
            , m_nwReply(nwReply)
        {}

        NcDownload(QNetworkReply *nwReply)
            : m_target(QString())
            , m_nwReply(nwReply)
        {}

        QString target() const
        {
            return m_target;
        }

        QNetworkReply *nwReply() const
        {
            return m_nwReply;
        }

        bool operator==(const NcDownload &toCompare) const {
            return m_nwReply == toCompare.nwReply();
        }

    private:
        QString m_target;
        QNetworkReply *m_nwReply;
    };

    QNetworkAccessManager m_nam;
    QVector<NcDownload> m_currentDownloads;
    bool saveToDisk(const QString &filename, QIODevice *data);
    QString buildPath(const QString &path, NextcloudAccount *account);
};
