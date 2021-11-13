// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "download.h"

#include "../models/accountmodel.h"

#include "QDesktopServices"
#include "QDebug"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QStandardPaths>
#include <KLocalizedString>

void Download::getFile(const QString &path, const QUrl &localPath, int accountId)
{
    auto accountService = AccountModel::getInstance();
    auto account = accountService->getAccountById(accountId);

    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/remote.php/webdav/" + path);

    account->get(endpoint, [this, path, localPath](QNetworkReply *reply) {
        QFile file(localPath.toLocalFile());
        const QFileInfo info(file);
        QDir parentDir;
        parentDir.mkpath(info.absolutePath());

        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Could not open" << localPath << "for writing" << path;
            Q_EMIT errorOccured(i18n("Couldn't save file, the destination is not writable."));
            return;
        }

        file.write(reply->readAll());
        file.close();
        Q_EMIT fileDownloaded(localPath.toLocalFile());
        // TODO call dbus dbus-send --session --print-reply --dest=org.freedesktop.FileManager1 --type=method_call /org/freedesktop/FileManager1 org.freedesktop.FileManager1.ShowItems array:string:"file:///home/me/path/to/folder/or/file" string:""
        return;
    });
}
