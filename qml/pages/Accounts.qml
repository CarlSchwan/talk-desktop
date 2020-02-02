import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: accounts
    allowedOrientations: Orientation.All

    SilicaListView {
        id: accountList
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Accounts")
        }

        delegate: ListItem {
            Label {
                text: name
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
            }
            onClicked: {
                if(account === -1) {
                    pageStack.push(Qt.resolvedUrl("./LegacyAddAccount.qml"), {
                        "onSuccessToRooms": accountList.count === 1
                    })
                }
            }
            menu: ContextMenu {
                hasContent: account !== -1
                MenuItem {
                    text: qsTr("Delete account")
                    onClicked: {
                        AccountService.deleteAccount(account)
                    }
                }
            }
        }

        model: AccountService

        VerticalScrollDecorator {}
    }
}
