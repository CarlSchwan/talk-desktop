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
    property int replyToId: -1;
    property string replyToMsg: "";
    readonly property string messageStyleSheet:
        "<style>" +
            "a:link { color: " + Theme.highlightColor + "; }" +
            ".highlight { color: " + Theme.highlightColor + "; }" +
            ".repliedTo {
                color: " + Theme.secondaryColor +";
                font-size: small;
                font-style: italic;
            }" +
        "</style>";
    readonly property string messageMention:
        "<strong class='{CLASS}'>{MENTION}</strong>";
    readonly property string messageRepliedTo:
        "<blockquote class='repliedTo'>{RTOMSG} <span class='repliedToActor'>({RTOACTOR})</span></blockquote>";

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            roomService.startPolling(token, accountId)
        } else if(status === PageStatus.Deactivating) {
            if(pageStack.currentPage.pageName !== "Participants") {
                roomService.stopPolling()
            }
        } else if(status === PageStatus.Inactive) {
            if(pageStack.currentPage.pageName !== "Participants") {
                messages.clear();
            }
            pageStack.popAttached()
        } else if(status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("./participants.qml"),
                {
                    token: room.token,
                    accountId: room.accountId,
                    textField: sendMessage
                }
            );
        }
    }

    function prepareMessage(message) {
        message.mid = message.id
        message.message = message.message.replace('{actor}', message.actorDisplayName)
        message.timeString = new Date(message.timestamp * 1000).toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'})
        message.dateString = new Date(message.timestamp * 1000).toLocaleDateString(undefined, {day: '2-digit', motnh: '2-digit'})
        message.message = escapeTags(message.message)
        message.message = handleMessageParameters(message.messageParameters, message.message)
        if(message.message === "{file}") {
            var actorSnippet = createMentionSnippet(message.messageParameters['actor']);

            message.message = actorSnippet + " " + qsTr("shared") + " " +
                    '<a rel="noopener noreferrer" href="' + message.messageParameters['file'].link + '">' +
                    message.messageParameters['file'].name + '</a>';
        } else {
            message.message = formatLinksRich(message.message)
        }

        message.repliedTo = ""
        if(message.parent) {
            if(message.parent.messageParameters) {
                message.parent.message = handleMessageParameters(message.parent.messageParameters, message.parent.message);
            }
            message.repliedTo = prepareRepliedTo(message);
        }

        delete message.messageParameters

        return message
    }

    function handleMessageParameters(parameters, message) {
        Object.keys(parameters).forEach(function(key) {
            if(key.substring(0, 8) === 'mention-') {
                var insertSnippet = createMentionSnippet(parameters[key]);
                message = message.replace('{' + key + '}', insertSnippet)
            }
        })
        return message
    }

    function escapeTags(text) {
        return text.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
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

    function prepareRepliedTo(message) {
        var quote = messageRepliedTo;
        quote = quote.replace('{RTOMSG}', stripTags(message.parent.message));
        quote = quote.replace('{RTOACTOR}', message.parent.actorDisplayName);
        return quote;
    }

    function stripTags(s) {
        return s.replace(/(<([^>]+)>)/ig,"")
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
        anchors {
            top: parent.top
            bottom: sendMessagePart.top
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
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

        delegate: ListItem {
            height: author.contentHeight + messageText.contentHeight + Theme.paddingMedium + ctxMenu.height

            Column {
                width: parent.width

                Label {
                    id: author
                    text: timeString + " · " + actorDisplayName + " · " + dateString
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
                    text: room.messageStyleSheet + repliedTo + message
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
            menu: ContextMenu {
                id: ctxMenu;
                container: chat
                MenuItem {
                    text: qsTr("Reply")
                    visible: isReplyable
                    onClicked: {
                        replyToId = mid
                        replyToMsg = stripTags(message)
                        sendMessage.focus = true
                    }
                }
                MenuItem {
                    text: qsTr("Mention")
                    visible: actorType == "users"
                    onClicked: {
                        sendMessage.text = sendMessage.text + " @" + actorId;
                    }
                }
                MenuItem {
                    text: qsTr("Copy text")
                    onClicked: Clipboard.text = stripTags(message)
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

    Column {
        id: sendMessagePart
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width

        Row {
            width: parent.width
            visible: replyToId != -1
            spacing: Theme.paddingSmall
            height: Theme.iconSizeSmall

            Separator {
                width: Theme.horizontalPageMargin
            }

            Icon {
                id: replyIndicator
                source: "image://theme/icon-s-repost"
                color: palette.secondaryHighlightColor
            }

            Label {
                width: parent.width - Theme.horizontalPageMargin * 2 - Theme.iconSizeSmall - Theme.paddingSmall * 3 - replyToClear.width
                id: replyTo
                text: replyToMsg
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: "NoWrap"
                elide: "ElideMiddle"
                color: Theme.secondaryHighlightColor
                height: parent.height
            }
            IconButton {
                height: Theme.iconSizeSmall
                width: Theme.iconSizeSmall
                id: replyToClear
                icon.source: "image://theme/icon-s-clear-opaque-cross"
                icon.color: palette.secondaryHighlightColor
                onClicked: {
                    replyToMsg = ""
                    replyToId = -1
                }
            }
        }

        Row {
            width: parent.width

            TextField {
                width: parent.width
                id: sendMessage
                placeholderText: "Write something excellent"
                EnterKey.enabled: text.length > 0
                EnterKey.onClicked: {
                    roomService.sendMessage(sendMessage.text, replyToId);
                    // FIXME: only clear text after it was send
                    sendMessage.text = ""
                    replyToId = -1
                }
                onClicked: chat.scrollToBottom()
            }
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
