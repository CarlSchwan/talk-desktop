// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

class SystemMessageTypeClass {
    Q_GADGET

public:
    enum Types {
        Dummy,
        ConversationCreated,
        ConversationRenamed,
        DescriptionRemoved,
        DescriptionSet,
        CallStarted,
        CallJoined,
        CallLeft,
        CallEnded,
        CallMissed,
        CallTried,
        ReadOnlyOff,
        ReadOnly,
        ListableNone,
        ListableUsers,
        ListableAll,
        LobbyNone,
        LobbyNonModerators,
        LobbyOpenToEveryone,
        GuestsAllowed,
        GuestsDisallowed,
        PasswordSet,
        PasswordRemoved,
        UserAdded,
        UserRemoved,
        ModeratorPromoted,
        ModeratorDemoted,
        GuestModeratorPromoted,
        GuestModeratorDemoted,
        MessageDeleted,
        FileShared,
        ObjectShared,
        MatterbridgeConfigAdded,
        MatterbridgeConfigEdited,
        MatterbridgeConfigRemoved,
        MatterbridgeConfigEnabled,
        MatterbridgeConfigDisabled,
        ClearedChat,
        Unknow, // 
    };
    Q_ENUM(Types)
};

using SystemMessageType = SystemMessageTypeClass::Types;

SystemMessageType systemMessageTypeFromString(const QString &string);
