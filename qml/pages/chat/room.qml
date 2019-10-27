import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: room
    allowedOrientations: Orientation.All

    property string token;
    property string roomName;
    property int accountId;
    property string accountUserId;
    readonly property string messageStyleSheet:
        "<style>" +
            "a:link { color: " + Theme.highlightColor + "; }" +
            ".highlight { color: " + Theme.highlightColor + "; }" +
        "</style>";
    readonly property string messageMention:
        "<strong class='{CLASS}'>{MENTION}</strong>";

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
        message.timeString = new Date(message.timestamp * 1000).toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'})
        Object.keys(message.messageParameters).forEach(function(key) {
            if(key.substring(0, 8) === 'mention-') {
                var insertSnippet = createMentionSnippet(message.messageParameters[key]);
                message.message = message.message.replace('{' + key + '}', insertSnippet)
            }
        })
        if(message.message === "{file}") {
            var actorSnippet = createMentionSnippet(message.messageParameters['actor']);

            message.message = actorSnippet + " " + qsTr("shared") + " " +
                    '<a rel="noopener noreferrer" href="' + message.messageParameters['file'].link + '">' +
                    message.messageParameters['file'].name + '</a>';
        } else {
            message.message = formatLinksRich(message.message)
        }

        return message
    }

    function formatLinksRich(content) {
        const urlRegex = /(\s|^)(https?:\/\/)?((?:[-A-Z0-9+_]+\.)+[-A-Z]+(?:\/[-A-Z0-9+&@#%?=~_|!:,.;()]*)*)(\s|$)/ig

        return content.replace(urlRegex, __linkReplacer)
    }

    function createMentionSnippet(messageParameters) {
        var mentionSnippet = room.messageMention.replace('{MENTION}', messageParameters.name)
        var useClass = ''
        if(messageParameters.id === room.accountUserId) {
            useClass = 'highlight'
        } else if(messageParameters.id === room.token
                  && messageParameters.type === 'call') {
            useClass = 'highlight'
        }

        return mentionSnippet.replace('{CLASS}', useClass)
    }

    function __linkReplacer(_, leadingSpace, protocol, url, trailingSpace) {
        var linkText = url
        if (!protocol) {
            protocol = 'https://'
        } else if (protocol === 'http://') {
            linkText = protocol + url
        }

        return leadingSpace
                + '<a rel="noopener noreferrer" href="'
                + protocol
                + url
                + '">'
                + linkText
                + '</a>'
                + trailingSpace
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
                    text: room.messageStyleSheet + message
                    textFormat: Text.RichText
                    height: contentHeight
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    font.pixelSize: Theme.fontSizeSmall
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    onLinkActivated: Qt.openUrlExternally(link)
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
            message = JSON.parse(message)
            messages.append(prepareMessage(message))
            chat.scrollToBottom()
        }
    }

}
