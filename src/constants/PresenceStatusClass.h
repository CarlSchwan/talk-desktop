// SPDX-FileCopyrightText: 2020 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>

#pragma once

#include <QObject>

class PresenceStatusClass {
    Q_GADGET

public:
    // as defined in https://github.com/nextcloud/spreed/blob/master/lib/Room.php
    enum Types {
        Offline ,
        Online,
        Away,
        DND,
        Invisible,
    };
    Q_ENUM(Types)

private:
    explicit PresenceStatusClass() {}
};

using PresenceStatus = PresenceStatusClass::Types;
