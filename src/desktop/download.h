// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2021 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-License-Identifier: GPL-3.0-or-late

#pragma once

#include <QObject>
#include <QQmlEngine>

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

public Q_SLOTS:
    void getFile(const QString &remotePath, const QUrl &localPath, int accountId);

Q_SIGNALS:
    void fileDownloaded(const QString &localFile);
    void errorOccured(const QString &error);
};
