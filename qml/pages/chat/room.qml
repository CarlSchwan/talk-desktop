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
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        //anchors.bottomMargin:  20
        width: parent.width
        boundsBehavior: Flickable.DragOverBounds


        header: PageHeader {
            id: header
            title: roomName
        }

        delegate: BackgroundItem {
            Label {
                text: message
                textFormat: Text.RichText;
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        }

        model: ListModel {
            id: messages
        }

        VerticalScrollDecorator {
            flickable: chat
        }
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
