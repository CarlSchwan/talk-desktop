import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Dialog {
    id: legacyAddAccount
    allowedOrientations: Orientation.All
    canAccept: false

    property var accData;
    property bool onSuccessToRooms;

    signal lgcyHostEntered(string host)

    onAccepted: {
        if(legacyAddAccount.onSuccessToRooms) {
            legacyAddAccount.acceptDestination = Qt.resolvedUrl("./chat/rooms.qml")
            legacyAddAccount.acceptDestinationAction = PageStackAction.Replace
        }

        AccountService.addAccount(accData.host, accData.loginName, accData.token, accData.userId)
    }

    SilicaFlickable {
        anchors.fill: parent

        DialogHeader { }

        Discovery {
            id: discovery;
            onDiscoverySuccessful: function(ncServer) {
                instance.text = ncServer
                busyHost.running = false
                loginName.enabled = true
                token.enabled = true
                testHost.enabled = true
                loginName.forceActiveFocus()
            }
            onDiscoveryFailed: {
                busyHost.running = false
                loginName.enabled = false
                token.enabled = false
                testHost.enabled = true
            }
            onCredentialsVerificationFailed: {
                busyCreds.running = false
                testHost.enabled = true
                loginName.forceActiveFocus()
            }
            onCredentialsVerificationSuccessful: function(host, loginName, token, userId) {
                busyCreds.running = false
                legacyAddAccount.canAccept = true
                legacyAddAccount.accData = {
                    "host": host,
                    "loginName": loginName,
                    "token": token,
                    "userId": userId,
                }
            }
        }

        Column {
            id: columnt
            width: parent.width
            spacing: Theme.paddingSmall

            PageHeader { title: qsTr("Add account") }

            TextField {
                id: instance
                focus: true
                label: qsTr("Enter a Nextcloud host")
                placeholderText: label
                width: parent.width
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    busyHost.running = true;
                    discovery.discoverInstance(instance.text);
                    testHost.enabled = false
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                id: testHost
                text: qsTr("Verify Host")
                onClicked: {
                    busyHost.running = true
                    discovery.discoverInstance(instance.text);
                    testHost.enabled = false
                }

                BusyIndicator {
                    id: busyHost
                    anchors.centerIn: testHost
                    running: false;
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
                onTextChanged: {
                    legacyAddAccount.canAccept = false
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
                    busyCreds   .running = true;
                    discovery.verifyCredentials(instance.text, loginName.text, token.text)
                }
                onTextChanged: {
                    legacyAddAccount.canAccept = false
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                id: testCredentials
                text: qsTr("Verify Credentials")
                enabled: loginName.text && token.text && !busyCreds.running
                onClicked: {
                    busyCreds.running = true
                    discovery.verifyCredentials(instance.text, loginName.text, token.text)
                }

                BusyIndicator {
                    anchors.centerIn: testCredentials
                    id: busyCreds
                }
            }

        }
    }
}
