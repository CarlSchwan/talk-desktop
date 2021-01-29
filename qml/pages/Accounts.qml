import QtQuick 2.6
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

        delegate: BackgroundItem {
            anchors.left: parent.left
            anchors.right: parent.right

            height: (accountLogo.height > instanceCol.height
                    ? accountLogo.height
                    : instanceCol.height) + Theme.paddingMedium + accountMenu.height

            ListItem {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Theme.horizontalPageMargin
                width: parent.width

                Image {
                    id: accountLogo
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    source: instanceLogo
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    fillMode: Image.PreserveAspectFit
                }

                Column {
                    id: instanceCol
                    anchors {
                        left: accountLogo.right
                        leftMargin: Theme.paddingMedium
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    width: parent.width - accountLogo.width - Theme.horizontalPageMargin * 2
                    height: accountName.paintedHeight + instanceLabel.height

                    Label {
                        id: instanceLabel
                        text: instanceName || ""
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        font.weight: Font.Light
                        truncationMode: TruncationMode.Fade
                        visible: (typeof (instanceName) !== "undefined") && instanceName != ""
                        height: visible ? paintedHeight : 0
                        color: Theme.secondaryColor
                    }

                    Label {
                        id: accountName
                        text: name
                        width: parent.width
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.DemiBold
                        truncationMode: TruncationMode.Fade
                    }
                }

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
                menu: ContextMenu {
                    id: accountMenu
                    hasContent: account !== -1
                    MenuItem {
                        text: qsTr("Delete account")
                        onClicked: {
                            AccountService.deleteAccount(account)
                        }
                    }
                }
            }
        }

        model: AccountService

        VerticalScrollDecorator {}
    }
}
