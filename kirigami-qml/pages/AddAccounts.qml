// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.9-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0 as Talk

Kirigami.Page {
    id: root
    property var accData;
    Talk.Discovery {
        id: discovery;
        onDiscoverySuccessful: function(ncServer) {
            instance.text = ncServer
            busyHost.running = false
            loginName.enabled = true
            token.enabled = true
            testHost.enabled = true
            loginName.forceActiveFocus()
            console.log("success")
        }
        onDiscoveryFailed: {
            busyHost.running = false
            loginName.enabled = false
            token.enabled = false
            testHost.enabled = true
            console.log("fail")
        }
        onCredentialsVerificationFailed: {
            console.log("fail")
            busyCreds.running = false
            testHost.enabled = true
            loginName.forceActiveFocus()
        }
        onCredentialsVerificationSuccessful: function(host, loginName, token, userId) {
            console.log("success")
            busyCreds.running = false
            acceptButton.enabled = true
            root.accData = {
                "host": host,
                "loginName": loginName,
                "token": token,
                "userId": userId,
            }
        }
    }
    ColumnLayout {
        anchors.centerIn: parent
        anchors.margins: Kirigami.Units.largeSpacing
        Kirigami.FormLayout {
            QQC2.TextField {
                id: instance
                Kirigami.FormData.label: i18n("Enter a Nextcloud host")
                onAccepted: {
                    busyHost.running = true;
                    discovery.discoverInstance(instance.text);
                    testHost.enabled = false
                }
            }
            QQC2.Button {
                id: testHost
                enabled: instance.text.length > 0
                text: i18n("Verify Host")
                onClicked: {
                    busyHost.running = true
                    discovery.discoverInstance(instance.text);
                    testHost.enabled = false
                }

                QQC2.BusyIndicator {
                    id: busyHost
                    anchors.centerIn: testHost
                    running: false;
                }
            }
            QQC2.TextField {
                id: loginName
                Kirigami.FormData.label: i18n("Enter login name")
                onAccepted: legacyAddAccount.canAccept = false
            }
            Kirigami.PasswordField {
                id: token
                Kirigami.FormData.label: i18n("Enter password")
                enabled: false
                onAccepted: {
                    busyCreds.running = true;
                    discovery.verifyCredentials(instance.text, loginName.text, token.text)
                }
                onTextChanged: acceptButton.enabled = false
            }
            QQC2.Button {
                id: testCredentials
                text: qsTr("Verify Credentials")
                enabled: loginName.text && token.text && !busyCreds.running
                onClicked: {
                    busyCreds.running = true
                    discovery.verifyCredentials(instance.text, loginName.text, token.text)
                }

                QQC2.BusyIndicator {
                    id: busyCreds
                    anchors.centerIn: testCredentials
                    running: false
                }
            }
            QQC2.Button {
                id: acceptButton
                text: i18n("Accept")
                onClicked: Talk.AccountService.addAccount(accData.host, accData.loginName, accData.token, accData.userId)
            }
        }
    }
}
