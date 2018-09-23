import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

Page {
    id: rooms
    allowedOrientations: Orientation.All

    onStatusChanged: {
        if(status === PageStatus.Activating) {
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
        }

        model:  RoomService {
            id: roomService
        }

        VerticalScrollDecorator {}
    }
}
