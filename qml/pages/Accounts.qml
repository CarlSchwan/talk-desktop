import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

Page {
    id: accounts
    allowedOrientations: Orientation.All

    Component {
        id: accountItemDelegate
        //property bool isAddAccount
        Item {
            Text {
                text: name
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        //contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        //Column {
          //  id: column
            //width: parent.width
            //anchors.fill: parent

            SilicaListView {
                id: accountList
                anchors.fill: parent
                //delegate: accountItemDelegate

                header: PageHeader {
                    title: qsTr("Accounts")
                }

                delegate: BackgroundItem {
                    Label {
                        text: name
                    }
                }

                model: AccountModel {
                    id: accountModel
                }
            }
        //}
    }
}
