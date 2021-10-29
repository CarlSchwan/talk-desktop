// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "systemmessagetype.h"

#include <QHash>

/*
* `conversation_created` - {actor} created the conversation
* `conversation_renamed` - {actor} renamed the conversation from "foo" to "bar"
* `description_set` - {actor} set the description to "Hello world"
* `description_removed` - {actor} removed the description
* `call_started` - {actor} started a call
* `call_joined` - {actor} joined the call
* `call_left` - {actor} left the call
* `call_ended` - Call with {user1}, {user2}, {user3}, {user4} and {user5} (Duration 30:23)
* `call_missed` - You missed a call from {user}
* `call_tried` - You tried to call {user}
* `read_only_off` - {actor} unlocked the conversation
* `read_only` - {actor} locked the conversation
* `listable_none` - {actor} limited the conversation to the current participants
* `listable_users` - {actor} opened the conversation accessible to registered users
* `listable_all` - {actor} opened the conversation accessible to registered and guest app users
* `lobby_timer_reached` - The conversation is now open to everyone
* `lobby_none` - {actor} opened the conversation to everyone
* `lobby_non_moderators` - {actor} restricted the conversation to moderators
* `guests_allowed` - {actor} allowed guests in the conversation
* `guests_disallowed` - {actor} disallowed guests in the conversation
* `password_set` - {actor} set a password for the conversation
* `password_removed` - {actor} removed the password for the conversation
* `user_added` - {actor} added {user} to the conversation
* `user_removed` - {actor} removed {user} from the conversation
* `moderator_promoted` - {actor} promoted {user} to moderator
* `moderator_demoted` - {actor} demoted {user} from moderator
* `guest_moderator_promoted` - {actor} promoted {user} to moderator
* `guest_moderator_demoted` - {actor} demoted {user} from moderator
* `message_deleted` - Message deleted by {actor} (Should not be shown to the user)
* `history_cleared` - {actor} cleared the history of the conversation
* `file_shared` - {file}
* `object_shared` - {object}
* `matterbridge_config_added` - {actor} set up Matterbridge to synchronize this conversation with other chats
* `matterbridge_config_edited` - {actor} updated the Matterbridge configuration
* `matterbridge_config_removed` - {actor} removed the Matterbridge configuration
* `matterbridge_config_enabled` - {actor} started Matterbridge
* `matterbridge_config_disabled` - {actor} stopped Matterbridge
*/
SystemMessageType systemMessageTypeFromString(const QString &string) {
    const QHash<QString, SystemMessageType> map = {
        {QLatin1String("conversation_created"), SystemMessageType::ConversationCreated},
        {QLatin1String("conversation_renamed"), SystemMessageType::ConversationRenamed},
        {QLatin1String("description_set"), SystemMessageType::DescriptionSet},
        {QLatin1String("description_removed"), SystemMessageType::DescriptionRemoved},
        {QLatin1String("call_started"), SystemMessageType::CallStarted},
        {QLatin1String("call_joined"), SystemMessageType::CallJoined},
        {QLatin1String("call_left"), SystemMessageType::CallLeft},
        {QLatin1String("call_ended"), SystemMessageType::CallEnded},
        {QLatin1String("call_missed"), SystemMessageType::CallMissed},
        {QLatin1String("call_tried"), SystemMessageType::CallTried},
        {QLatin1String("read_only_off"), SystemMessageType::ReadOnlyOff},
        {QLatin1String("read_only"), SystemMessageType::ReadOnly},
        {QLatin1String("listable_none"), SystemMessageType::ListableNone},
        {QLatin1String("listable_users"), SystemMessageType::ListableUsers},
        {QLatin1String("listable_all"), SystemMessageType::ListableAll},
        {QLatin1String("lobby_none"), SystemMessageType::LobbyNone},
        {QLatin1String("lobby_non_moderators"), SystemMessageType::LobbyNonModerators},
        {QLatin1String("lobby_timer_reached"), SystemMessageType::LobbyOpenToEveryone},
        {QLatin1String("guests_allowed"), SystemMessageType::GuestsAllowed},
        {QLatin1String("guests_disallowed"), SystemMessageType::GuestsDisallowed},
        {QLatin1String("password_set"), SystemMessageType::PasswordSet},
        {QLatin1String("password_removed"), SystemMessageType::PasswordRemoved},
        {QLatin1String("user_added"), SystemMessageType::UserAdded},
        {QLatin1String("user_removed"), SystemMessageType::UserRemoved},
        {QLatin1String("moderator_promoted"), SystemMessageType::ModeratorPromoted},
        {QLatin1String("moderator_demoted"), SystemMessageType::ModeratorDemoted},
        {QLatin1String("guest_moderator_promoted"), SystemMessageType::GuestModeratorPromoted},
        {QLatin1String("guest_moderator_demoted"), SystemMessageType::GuestModeratorDemoted},
        {QLatin1String("message_deleted"), SystemMessageType::MessageDeleted},
        {QLatin1String("file_shared"), SystemMessageType::FileShared},
        {QLatin1String("object_shared"), SystemMessageType::ObjectShared},
        {QLatin1String("matterbridge_config_added"), SystemMessageType::MatterbridgeConfigAdded},
        {QLatin1String("matterbridge_config_edited"), SystemMessageType::MatterbridgeConfigEdited},
        {QLatin1String("matterbridge_config_removed"), SystemMessageType::MatterbridgeConfigRemoved},
        {QLatin1String("matterbridge_config_enabled"), SystemMessageType::MatterbridgeConfigEnabled},
        {QLatin1String("matterbridge_config_disabled"), SystemMessageType::MatterbridgeConfigDisabled},
        {QLatin1String("history_cleared"), SystemMessageType::ClearedChat}
    };

    const auto result = map.find(string);
    if (result == map.cend()) {
        return SystemMessageType::Unknow;
    }
    return *result;
}

