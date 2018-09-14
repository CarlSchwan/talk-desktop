import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

Page {
    id: accounts
    allowedOrientations: Orientation.All

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            accountModel.loadAccounts()
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        VerticalScrollDecorator {}

        SilicaListView {
            id: accountList
            anchors.fill: parent

            header: PageHeader {
                title: qsTr("Accounts")
            }

            delegate: BackgroundItem {
                Label {
                    text: name
                }
                onClicked: {
                    if(account === -1) {
                        pageStack.push(Qt.resolvedUrl("./LegacyAddAccount.qml"), {})
                    }
                }
            }

            model: AccountModel {
                id: accountModel
            }
        }
    }
}
