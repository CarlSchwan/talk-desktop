// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import harbour.nextcloud.talk 1.0

Kirigami.ScrollablePage {
    ColumnLayout {
        Kirigami.FormLayout {
            QQC2.CheckBox {
                Kirigami.FormData.label: i18n("General settings:")
                text: i18n("Close to system tray")
                checked: Config.systemTray
                visible: Controller.supportSystemTray
                enabled: !Config.isSystemTrayImmutable
                onToggled: {
                    Config.systemTray = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                // TODO: When there are enough notification and timeline event
                // settings, make 2 separate groups with FormData labels.
                Kirigami.FormData.label: i18n("Notifications and events:")
                text: i18n("Show notifications")
                checked: Config.showNotifications
                enabled: !Config.isShowNotificationsImmutable
                onToggled: {
                    Config.showNotifications = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Show leave and join events")
                checked: Config.showLeaveJoinEvent
                enabled: !Config.isShowLeaveJoinEventImmutable
                onToggled: {
                    Config.showLeaveJoinEvent = checked
                    Config.save()
                }
            }
        }
    }
}
