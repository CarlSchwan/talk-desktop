// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "download.h"
#include "QDesktopServices"
#include "QDebug"
#include <QDir>
#include "QFile"
#include <QFileInfo>
#include <QNetworkReply>
#include <QMetaMethod>
#include <QStandardPaths>
#include "requestfactory.h"

void Download::getFile(const QString &path, int accountId)
{
    auto accountService = AccountModel::getInstance();
    auto account = accountService->getAccountById(accountId);

    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/remote.php/webdav/" + path);

    if(!isSignalConnected(QMetaMethod::fromSignal(&QNetworkAccessManager::finished))) {
        connect(&m_nam, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));
    }

    QNetworkRequest request = RequestFactory::getRequest(endpoint, account);
    QNetworkReply *reply = m_nam.get(request);

    QString target = buildPath(path, account);

    NcDownload dl = NcDownload(target, reply);
    m_currentDownloads.append(dl);
}

bool Download::fileExists(const QString &path, int accountId)
{
    const auto accountService = AccountModel::getInstance();
    const auto account = accountService->getAccountById(accountId);
    const QString target = buildPath(path, account);
    const QFile file(target);
    return file.exists();
}

QString Download::filePath(const QString &path, int accountId)
{
    const auto accountService = AccountModel::getInstance();
    const auto account = accountService->getAccountById(accountId);
    return buildPath(path, account);
}

QString Download::buildPath(const QString &path, NextcloudAccount *account)
{
    return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first()
            + "/Nextcloud Talk/"
            + account->name() + "/"
            + path;
}

void Download::downloadFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode != 200) {
        qDebug() << "Download of " << reply->url() << "did not succeed with code" << statusCode;
        return;
    }

    int i = m_currentDownloads.indexOf(NcDownload(reply));
    if(i == -1) {
        qDebug() << "Cannot recognize download job for" << reply->url();
        return;
    }
    NcDownload dl = m_currentDownloads.at(i);

    if (saveToDisk(dl.target(), reply)) {
        printf("Download of %s succeeded (saved to %s)\n",
               reply->url().toEncoded().constData(), qPrintable(dl.target()));
    }
    m_currentDownloads.removeAll(reply);
    reply->deleteLater();

    emit fileDownloaded(dl.target());
}

bool Download::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    const QFileInfo info(file);
    QDir parentDir;
    parentDir.mkpath(info.absolutePath());

    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}
