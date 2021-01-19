#ifndef CONVERSATIONTYPECLASS_H
#define CONVERSATIONTYPECLASS_H

#pragma once

#include <QObject>

class ConversationTypeClass {
    Q_GADGET

public:
    // as defined in https://github.com/nextcloud/spreed/blob/master/lib/Room.php
    enum Types {
        Unknown = -1,
        OneToOne = 1,
        Group = 2,
        Public = 3,
        Changelog = 4,
    };
    Q_ENUM(Types)

private:
    explicit ConversationTypeClass() {}
};

typedef ConversationTypeClass::Types ConversationType;

#endif // CONVERSATIONTYPECLASS_H
