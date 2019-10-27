import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: rooms
    allowedOrientations: Orientation.All

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            console.debug("Loading rooms")
            roomService.loadRooms()
        }
    }



    SilicaListView {
        PullDownMenu {
             MenuItem {
                 text: qsTr("Accounts")
                 onClicked: pageStack.push(Qt.resolvedUrl("../Accounts.qml"), {})
             }
         }

        BusyIndicator {
            id: roomSync
            running: false
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
            onModelAboutToBeReset: roomSync.running = true
            onModelReset: roomSync.running = false
        }

        VerticalScrollDecorator {}
    }
}
