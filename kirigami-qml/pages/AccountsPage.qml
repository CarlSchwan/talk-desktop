// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import harbour.nextcloud.talk 1.0 as Talk

import org.kde.kirigami 2.15 as Kirigami

Kirigami.Page {
    title: i18n("Accounts")

    leftPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    topPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    bottomPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    rightPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0

    actions.main: Kirigami.Action {
        text: i18n("Add an account")
        icon.name: "list-add-user"
        onTriggered: pageStack.layers.push("qrc:/pages/AddAccounts.qml")
        visible: !pageSettingStack.wideMode
    }

    ColumnLayout {
        anchors.fill: parent

        Connections {
            target: pageSettingStack
            onWideModeChanged: scroll.background.visible = pageSettingStack.wideMode
        }

        Controls.ScrollView {
            id: scroll
            Component.onCompleted: background.visible = pageSettingStack.wideMode

            Layout.fillWidth: true
            Layout.fillHeight: true

            Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff
            ListView {
                clip: true
                model: Talk.AccountService
                delegate: Kirigami.SwipeListItem {
                    leftPadding: 0
                    rightPadding: 0
                    Kirigami.BasicListItem {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        text: model.name
                        subtitle: model.instanceName
                        icon: model.instanceLogo.length > 0 ? model.instanceLogo : "im-user"
                    }
                    actions: [
                        Kirigami.Action {
                            text: i18n("Edit this account")
                            iconName: "document-edit"
                            onTriggered: {
                                userEditSheet.connection = model.connection
                                userEditSheet.open()
                            }
                        },
                        Kirigami.Action {
                            text: i18n("Logout")
                            iconName: "im-kick-user"
                            onTriggered: Talk.AccountService.deleteAccount(model.account)
                        }
                    ]
                }
            }
        }


        RowLayout {
            Item {
                Layout.fillWidth: true
            }
            Controls.Button {
                visible: pageSettingStack.wideMode
                text: i18n("Add an account")
                icon.name: "list-add-user"
                onClicked: pageStack.layers.push("qrc:/pages/AddAccounts.qml")
            }
        }
    }

    Kirigami.OverlaySheet {
        id: userEditSheet

        property var connection

        header: Kirigami.Heading {
            text: i18n("Edit Account")
        }

        Kirigami.FormLayout {
            RowLayout {
                Kirigami.Avatar {
                    id: avatar
                    source: userEditSheet.connection && userEditSheet.connection.localUser.avatarMediaId ? ("image://mxc/" + userEditSheet.connection.localUser.avatarMediaId) : ""

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        property var fileDialog: null;
                        onClicked: {
                            if (fileDialog != null) {
                                return;
                            }

                            fileDialog = openFileDialog.createObject(Controls.ApplicationWindow.Overlay)

                            fileDialog.chosen.connect(function(receivedSource) {
                                mouseArea.fileDialog = null;
                                if (!receivedSource) {
                                    return;
                                }
                                parent.source = receivedSource;
                            });
                            fileDialog.onRejected.connect(function() {
                                mouseArea.fileDialog = null;
                            });
                            fileDialog.open();
                        }
                    }
                }
                Controls.Button {
                    visible: avatar.source.length !== 0
                    icon.name: "edit-clear"

                    onClicked: avatar.source = ""
                }
                Kirigami.FormData.label: i18n("Avatar:")
            }
            Controls.TextField {
                id: name
                text: userEditSheet.connection ? userEditSheet.connection.localUser.displayName : ""
                Kirigami.FormData.label: i18n("Name:")
            }
            Controls.TextField {
                id: currentPassword
                Kirigami.FormData.label: i18n("Current Password:")
                echoMode: TextInput.Password
            }
            Controls.TextField {
                id: newPassword
                Kirigami.FormData.label: i18n("New Password:")
                echoMode: TextInput.Password

            }
            Controls.TextField {
                id: confirmPassword
                Kirigami.FormData.label: i18n("Confirm new Password:")
                echoMode: TextInput.Password
            }

            RowLayout {
                Controls.Button {
                    text: i18n("Save")
                    onClicked: {
                    }
                }
                Controls.Button {
                    text: i18n("Cancel")
                    onClicked: {
                        userEditSheet.close()
                        avatar.source = userEditSheet.connection.localUser.avatarMediaId ? ("image://mxc/" + userEditSheet.connection.localUser.avatarMediaId) : ""
                        currentPassword.text = ""
                        newPassword.text = ""
                        confirmPassword.text = ""
                    }
                }
            }
        }
    }
}
