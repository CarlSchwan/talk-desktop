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
        message.message = message.message.replace('{actor}', message.actorDisplayName)
        message.timeString = new Date(message.timestamp).toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'})
        return message
    }

    SilicaListView {
        id: chat
        anchors.top: parent.top
        anchors.bottom: sendMessagePart.top
        width: parent.width
        height: parent.height - sendMessage.height
        contentHeight: height
        boundsBehavior: Flickable.DragOverBounds
        quickScroll: true
        quickScrollAnimating: true
        clip: true

        header: PageHeader {
            id: header
            title: roomName
        }
        headerPositioning: ListView.PullBackHeader

        delegate: BackgroundItem {
            height: author.contentHeight + messageText.contentHeight + Theme.paddingMedium
            Column {
                width: parent.width

                Label {
                    id: author
                    text: timeString + " · " + actorDisplayName
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
                    textFormat: Text.RichText
                    height: contentHeight
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

    Row {
        id: sendMessagePart
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        TextField {
            width: parent.width
            id: sendMessage
            placeholderText: "Write something excellent"
            EnterKey.enabled: text.length > 0
            EnterKey.onClicked: {
                roomService.sendMessage(sendMessage.text);
                // FIXME: only clear text after it was send
                sendMessage.text = ""
            }
            onClicked: chat.scrollToBottom()
        }
    }

    RoomService {
        id: roomService
    }

    Connections {
        target: roomService
        onNewMessage: {
            console.log(message)
            message = JSON.parse(message)
            messages.append(prepareMessage(message))
            chat.scrollToBottom()
        }
    }

}
