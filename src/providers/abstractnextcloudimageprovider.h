// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QNetworkRequest>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QSize>
#include <QThreadPool>

class NextcloudAccount;

class AbstractNextcloudImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize);
    virtual QNetworkRequest getRequest(const QString &subject, NextcloudAccount *account, const QSize &requestedSize) = 0;

private:
    NextcloudAccount *accountFromId(const QString &id);
    QThreadPool pool;
};

