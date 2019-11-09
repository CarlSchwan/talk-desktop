import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: rooms
    allowedOrientations: Orientation.All

    Timer {
        id: roomPolling
        interval: rooms.visible ? 5000 : 30000;
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: {
            roomService.loadRooms()
        }
    }

    SilicaListView {
        PullDownMenu {
            id: pulley
             MenuItem {
                 text: qsTr("Accounts")
                 onClicked: pageStack.push(Qt.resolvedUrl("../Accounts.qml"), {})
             }
         }

        id: roomList
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Conversations")
        }

        delegate: BackgroundItem {
            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingMedium
                }
                spacing: width - (roomName.width + unreadCounter.width)
                Label {
                    id: roomName
                    text: name
                }

                Label {
                    id: unreadCounter
                    text: unreadMessages === 0 ? ""
                            : unreadMessages > 99 ? ">99"
                            : unreadMessages
                    color: unreadMention ? Theme.highlightColor : Theme.primaryColor
                }
            }
            onClicked: {
                pageStack.push(Qt.resolvedUrl("./room.qml"),
                               {
                                   "token": token,
                                   "roomName": name,
                                   "accountId": accountId,
                                   "accountUserId": accountUserId,
                               }
                );
            }
        }

        model:  RoomService {
            id: roomService
            onModelAboutToBeReset: pulley.busy = true
            onModelReset: pulley.busy = false
        }

        VerticalScrollDecorator {}
    }
}
