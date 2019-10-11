import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

Page {
    id: room
    allowedOrientations: Orientation.All

    property string token;
    property string roomName;
    property int accountId;

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            roomService.startPolling(token, accountId)
        } else if(status === PageStatus.Deactivating) {
            roomService.stopPolling()
        } else if(status === PageStatus.Inactive) {
            messages.clear();
        }
    }

    SilicaListView {
        id: chat
        anchors.fill: parent

        header: PageHeader {
            id: header
            title: roomName
        }

        delegate: BackgroundItem {
            Label {
                text: message
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
            }
        }

        model: ListModel {
            id: messages
        }

        VerticalScrollDecorator {}
    }

    RoomService {
        id: roomService
    }

    Connections {
        target: roomService
        onNewMessage: {
            console.log(message)
            messages.append({'message': message})
        }
    }

}
