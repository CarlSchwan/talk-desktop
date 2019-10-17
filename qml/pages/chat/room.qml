import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

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

    function prepareMessage(message) {
        message.message = message.message.replace('{actor}', message.actorDisplayName);
        return message
    }


    SilicaListView {
        id: chat
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width
        boundsBehavior: Flickable.DragOverBounds


        header: PageHeader {
            id: header
            title: roomName
        }

        delegate: BackgroundItem {
            Column {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
                height: author.height + messageText.height;

                Label {
                    id: author
                    text: actorDisplayName
                    textFormat: Text.PlainText;
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                Label {
                    id: messageText
                    text: message
                    textFormat: Text.RichText;
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
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
            message = JSON.parse(message);
            messages.append(prepareMessage(message));
        }
    }

}
