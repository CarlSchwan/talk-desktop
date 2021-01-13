import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: rooms
    allowedOrientations: Orientation.All

    property string lastUpdate: ""

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
             MenuItem {
                 text: qsTr("Last update: ") + lastUpdate
                 enabled: false
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
                    rightMargin: Theme.paddingMedium
                    leftMargin: 0;
                }
                spacing: width - (nameRow.width + unreadCounter.width)
                Row {
                    id: nameRow

                    Rectangle {
                        id: accountIndicator
                        width: Theme.paddingSmall / 3
                        height: parent.height
                        color: primaryColor
                    }

                    Label {
                        id: roomName
                        text: name
                        leftPadding: Theme.paddingSmall;
                    }
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
            onDataChanged: {
                lastUpdate = new Date().toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'});
            }
        }

        VerticalScrollDecorator {}
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        running: lastUpdate === ""
        anchors.centerIn: parent
    }
}
