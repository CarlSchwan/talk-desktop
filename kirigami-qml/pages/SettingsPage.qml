// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0

Kirigami.Page {
    title: i18n("Settings")
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0

    onBackRequested: {
        if (pageSettingStack.depth > 1 && !pageSettingStack.wideMode && pageSettingStack.currentIndex !== 0) {
            event.accepted = true;
            pageSettingStack.pop();
        }
    }

    Kirigami.PageRow {
        id: pageSettingStack
        anchors.fill: parent
        columnView.columnWidth: Kirigami.Units.gridUnit * 12
        initialPage: Kirigami.ScrollablePage {
            bottomPadding: 0
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            ListView {
                Component.onCompleted: if (pageSettingStack.wideMode) {
                    actions[0].trigger();
                }
                property list<Kirigami.Action> actions: [
                    Kirigami.Action {
                        text: i18n("General")
                        icon.name: "org.kde.neochat"
                        onTriggered: pageSettingStack.push("qrc:/pages/GeneralSettingsPage.qml")
                    },
                    Kirigami.Action {
                        text: i18n("Appearance")
                        icon.name: "preferences-desktop-theme-global"
                        onTriggered: pageSettingStack.push("qrc:/pages/AppearanceSettingsPage.qml")
                    },
                    Kirigami.Action {
                        text: i18n("Account")
                        icon.name: "preferences-system-users"
                        onTriggered: pageSettingStack.push("qrc:/pages/AccountsPage.qml")
                    }
                ]
                model: actions
                delegate: Kirigami.BasicListItem {
                    action: modelData
                }
            }
        }
    }
}
