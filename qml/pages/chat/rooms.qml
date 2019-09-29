import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

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
        id: roomList
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Rooms")
        }

        delegate: BackgroundItem {
            Label {
                text: name
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
            }
            onClicked: pageStack.push(Qt.resolvedUrl("./room.qml"), {"room": roomList.indexAt(index)})
        }

        model:  RoomService {
            id: roomService
        }

        VerticalScrollDecorator {}
    }
}
