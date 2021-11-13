// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstractnextcloudimageprovider.h"

class AvatarProvider : public AbstractNextcloudImageProvider
{
public:
    AvatarProvider();
    QNetworkRequest getRequest(const QString &subject, NextcloudAccount *account, const QSize &requestedSize);
};
