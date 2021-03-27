#ifndef PRESENCESTATUSCLASS_H
#define PRESENCESTATUSCLASS_H

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

typedef PresenceStatusClass::Types PresenceStatus;

#endif // PRESENCESTATUSCLASS_H
