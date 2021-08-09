// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.9-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0
import '../../components/'

Kirigami.ScrollablePage {
    id: room

    title: roomName

    required property var roomService

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
        "</style>";
    readonly property string messageMention:
        "<strong class='{CLASS}'>{MENTION}</strong>";
    readonly property string messageRepliedTo:
        "{RTOMSG} ({RTOACTOR})";

    /*onStatusChanged: {
        if(status === PageStatus.Activating) {
            if(!roomService.isPolling(token, accountId)) {
                // do not re-enable when returning from participants
                roomService.startPolling(token, accountId)
            }
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
    }*/

    function prepareMessage(message) {
        message._lastOfActorGroup = true
        message._firstOfActorGroup = true
        message._type = "posting"
        message._mid = message.id

        message.message = message.message.replace('{actor}', message.actorDisplayName)
        message.timeString = new Date(message.timestamp * 1000).toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'})
        message.dateString = new Date(message.timestamp * 1000).toLocaleDateString(undefined, {day: '2-digit', motnh: '2-digit'})
        message.message = escapeTags(message.message)
        message.message = message.message.replace(/\n/g, '<br/>')
        message.message = handleMessageParameters(message.messageParameters, message.message)
        if(message.message === "{file}") {
            message._type = "file"
            message._fileId = message.messageParameters.file.id
            message._fileName = message.messageParameters.file.name
            message._filePath = message.messageParameters.file.path

            var actorSnippet = createMentionSnippet(message.messageParameters['actor']);
            var path = message.messageParameters['file'].path

            message.message = actorSnippet + " " + qsTr("shared") + " " +
                    '<a rel="noopener noreferrer" ' +
                    'href="javascript:DownloadService.getFile(\"' + path + "\", " + accountId + ')">' +
                    message.messageParameters['file'].name + '</a>';
        } else {
            message.message = formatLinksRich(message.message)
        }

        message.repliedTo = {
            author: "",
            message: ""
        }

        if(message.parent) {
            if(message.parent.messageParameters) {
                message.parent.message = handleMessageParameters(message.parent.messageParameters, message.parent.message);
            }
            message.repliedTo.author = message.parent.actorDisplayName
            message.repliedTo.message = stripTags(message.parent.message)
        }

        return message
    }

    function handleMessageParameters(parameters, message) {
        Object.keys(parameters).forEach(function(key) {
            // system message "You added {user}"
            if(key === 'user') {
                message = message.replace('{' + key + '}', parameters[key].name)
            }
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

    function updateLastOfActor(message) {
        if(messages.count > 0) {
            var previousMessage = messages.get(messages.count - 1)
            if(previousMessage && previousMessage.actorId === message.actorId) {
                previousMessage._lastOfActorGroup = false
            }
        }
    }

    function updateFirstOfActor(message) {
        if(messages.count > 0) {
            var previousMessage = messages.get(messages.count - 1)
            if(previousMessage && previousMessage.actorId === message.actorId) {
                message._firstOfActorGroup = false
            }
        }
    }

    ListView {
        id: chat
        delegate: TimelineContainer {
            id: messageContainer
            width: ListView.view.width
            isLoaded: true
            innerObject: TextDelegate {
                Layout.fillWidth: Config.compactLayout
                Layout.maximumWidth: messageContainer.bubbleMaxWidth
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
            }
        }
        model: room.roomService.messageModel
        /*delegate: ListItem {

            height: author.contentHeight
                    + repliedToAuthor.height
                    + repliedToText.height
                    + messageText.contentHeight
                    + Theme.paddingLarge
                    + ctxMenu.height
                    + filePreview.height

            Row {
                spacing: Theme.paddingSmall

                Avatar {
                    id: avatar
                    account: accountId
                    user: actorId
                    anchors.bottom: parent.bottom
                    opacity: _lastOfActorGroup ? 100 : 0
                }

                Column {
                    width: chat.width - avatar.width - Theme.paddingSmall
                    spacing: Theme.paddingSmall

                    Label {
                        id: author
                        text: {
                            if(_firstOfActorGroup) {
                                return timeString + " · " + actorDisplayName + " · " + dateString
                            }
                            return timeString
                        }
                        textFormat: Text.PlainText;
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        font.pixelSize: Theme.fontSizeTiny
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }

                    Row {
                        visible: repliedTo.message !== ""
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        Rectangle {
                            color: Theme.secondaryColor
                            width: 2
                            height: repliedToAuthor.height + repliedToText.height
                        }
                        Column {
                            width: chat.width - avatar.width - Theme.paddingSmall - 2
                            Label {
                                id: repliedToAuthor
                                width: parent.width
                                text: repliedTo.author
                                textFormat: Text.PlainText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                }
                                leftPadding: Theme.paddingSmall
                                font.pixelSize: Theme.fontSizeExtraSmall
                                wrapMode: "NoWrap"
                                elide: "ElideMiddle"
                                visible: repliedTo.author !== ""
                                color: Theme.secondaryColor
                                font.italic: true
                                height: visible ? contentHeight : 0
                            }
                            Label {
                                id: repliedToText
                                text: repliedTo.message
                                textFormat: Text.PlainText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                }
                                leftPadding: Theme.paddingSmall
                                font.pixelSize: Theme.fontSizeExtraSmall
                                wrapMode: "NoWrap"
                                elide: "ElideMiddle"
                                visible: repliedTo.message !== ""
                                color: Theme.secondaryColor
                                font.italic: true
                                height: visible ? contentHeight : 0
                            }
                        }
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
                    FilePreview {
                        id: filePreview
                        account: accountId
                        fileId: (_type === "file" && _fileId) ? _fileId : -1
                        filePath: (_type === "file" && _filePath) ? _filePath : ""
                        visible: _type === "file"
                        size: _type === "file" ? Theme.itemSizeHuge : 0
                        height: _type === "file" ? Theme.itemSizeHuge : 0
                    }
                }

            }

            menu: ContextMenu {
                id: ctxMenu;
                container: chat
                MenuItem {
                    text: qsTr("Reply")
                    visible: isReplyable ? true : false
                    onClicked: {
                        replyToId = _mid
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

        onCountChanged: {
            // when previous last item is fully visible scroll to end
            var isLastVisible = (chat.currentItem.y + chat.currentItem.height) >= chat.contentY
                && (chat.currentItem.y + chat.currentItem.height) <= (chat.contentY + height)
            if(chat.currentIndex == 0 || isLastVisible) {
                var newIndex = chat.count - 1;
                chat.positionViewAtEnd();
                chat.currentIndex = newIndex;
            }
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

            TextArea {
                id: sendMessage
                width: parent.width - sendIcon.width
                placeholderText: "Write something excellent"
                wrapMode: TextEdit.WordWrap
                EnterKey.enabled: text.length > 0
            }
            IconButton {
                id: sendIcon
                anchors.top: sendMessage.top
                anchors.bottom: sendMessage.bottom
                icon.source: "image://theme/icon-m-send?" + sendMessage.color
                Behavior on icon.source { FadeAnimation {} }
                opacity: sendMessage.text.length > 0 ? 1.0 : 0.3
                Behavior on opacity { FadeAnimation {} }
                onClicked: {
                    roomService.sendMessage(sendMessage.text, replyToId);
                    // FIXME: only clear text after it was send
                    sendMessage.text = ""
                    replyToId = -1
                }
            }
        }
        */
    }
}