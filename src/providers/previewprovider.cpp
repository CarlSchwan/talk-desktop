// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewprovider.h"
#include "nextcloudaccount.h"
#include <QNetworkRequest>
#include <QUrlQuery>

PreviewProvider::PreviewProvider()
    : AbstractNextcloudImageProvider()
{
}

QNetworkRequest PreviewProvider::getRequest(const QString &subject, NextcloudAccount *account, const QSize &requestedSize)
{
    const QUrlQuery urlQuery({
        {QStringLiteral("fileId"), subject},
        {QStringLiteral("x"), QString::number(requestedSize.width())},
        {QStringLiteral("y"), QString::number(requestedSize.height())}
    });

    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/core/preview");
    endpoint.setQuery(urlQuery);

    return account->setupRequest(endpoint);
}
