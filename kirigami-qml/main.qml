// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import harbour.nextcloud.talk 1.0 as Talk
import org.kde.kirigami 2.15 as Kirigami

Kirigami.ApplicationWindow {
    Component.onCompleted: {
        Talk.AccountService.loadAccounts();
        if (Talk.AccountService.rowCount() === 1) {
            pageStack.push("qrc:/pages/AddAccounts.qml")
        } else {
            pageStack.push("qrc:/pages/chat/rooms.qml")
        }
    }
}
