// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0 as Talk

Kirigami.Page {
    id: accounts
    title: i18n("Accounts")

    ListView {
        id: accountList
        delegate: Kirigami.SwipeListItem {
            leftPadding: Kirigami.Units.largeSpacing * 2
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            actions: Kirigami.Action {
                visible: account !== -1
                text: i18n("Delete account")
                onTriggered: AccountService.deleteAccount(account)
                icon.name: "delete"
            }

            contentItem: Kirigami.BasicListItem {
                leading: Image {
                    id: accountLogo
                    source: instanceLogo
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    fillMode: Image.PreserveAspectFit
                }
                label: name
                subtitle: instanceName || ""

                onClicked: {
                    if(account === -1) {
                        pageStack.push(Qt.resolvedUrl("./LegacyAddAccount.qml"), {
                            "onSuccessToRooms": accountList.count === 1
                        })
                    } else {
                        pageStack.push(Qt.resolvedUrl("accounts/AccountSettings.qml"), {
                            "accountIndex": AccountService.index(index, 0)
                        })
                    }
                }
            }
        }

        model: Talk.AccountService
    }
}
