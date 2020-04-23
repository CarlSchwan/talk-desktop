#ifndef DOWNLOAD_H
#define DOWNLOAD_H

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
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        return new Download;
    }

public slots:
    void getFile(QString path, int accountId);
    bool fileExists(QString path, int accountId);
    QString filePath(QString path, int accountId);

signals:
    void fileDownloaded(QString localFile);

private slots:
    void downloadFinished(QNetworkReply *reply);

private:
    struct NcDownload
    {
        NcDownload() {}
        NcDownload (QString target, QNetworkReply *nwReply) {
            this->target = target;
            this->nwReply = nwReply;
        }
        NcDownload (QNetworkReply *nwReply) {
            this->target = "";
            this->nwReply = nwReply;
        }
        QString target;
        QNetworkReply *nwReply;

        bool operator==(const NcDownload &toCompare) const {
            return nwReply == toCompare.nwReply;
        }
    };
    QNetworkAccessManager m_nam;
    QVector<NcDownload> m_currentDownloads;
    bool saveToDisk(const QString &filename, QIODevice *data);
    QString buildPath(QString path, NextcloudAccount account);
};

#endif // DOWNLOAD_H
