import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: legacyAddAccount
    allowedOrientations: Orientation.All

    signal lgcyHostEntered(string host)

    SilicaFlickable {
        anchors.fill: parent

        Discovery {
            id: discovery;
            onDiscoverySuccessful: function(ncServer) {
                instance.text = ncServer
                busy.running = false
                loginName.enabled = true
                token.enabled = true
                loginName.forceActiveFocus()
            }
            onDiscoveryFailed: {
                busy.running = false
                loginName.enabled = false
                token.enabled = false
            }
            onCredentialsVerificationFailed: {
                busy.running = false
                loginName.forceActiveFocus()
            }
            onCredentialsVerificationSuccessful: function(host, loginName, token, userId) {
                accountService.addAccount(host, loginName, token, userId)
                pageStack.pop()
            }
        }

        AccountModel {
            id: accountService;
        }

        BusyIndicator {
            id: busy
            running: false;
            // FIXME: that'S not what I want. I wont glowing upper edge.
        }

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Add account") }

            TextField {
                id: instance
                focus: true
                label: qsTr("Enter a Nextcloud host")
                placeholderText: label
                text: "https://o.schiwon.me"
                width: parent.width
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    busy.running = true;
                    discovery.discoverInstance(instance.text);
                }
            }

            TextField {
                id: loginName
                focus: true
                label: qsTr("Enter login name")
                placeholderText: label
                text: ""
                width: parent.width
                enabled: false
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    token.forceActiveFocus()
                }
            }

            PasswordField {
                id: token
                focus: true
                label: qsTr("Enter app password")
                placeholderText: label
                text: ""
                width: parent.width
                enabled: false
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    busy.running = true;
                    discovery.verifyCredentials(instance.text, loginName.text, token.text)
                }
            }
        }
    }
}
