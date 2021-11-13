// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avatarprovider.h"
#include "nextcloudaccount.h"

#include <QNetworkRequest>

AvatarProvider::AvatarProvider() : AbstractNextcloudImageProvider()
{
}

QNetworkRequest AvatarProvider::getRequest(const QString &subject, NextcloudAccount *account, const QSize &requestedSize)
{
    QUrl endpoint = QUrl(account->host());
    endpoint.setPath(endpoint.path() + "/avatar/" + subject + "/" + QString::number(requestedSize.height()));
    return account->setupRequest(endpoint);
}
