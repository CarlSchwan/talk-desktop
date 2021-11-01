// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.9-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0
import '../../components/'
import '../../ChatBox'

Kirigami.ScrollablePage {
    id: room

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

    ListView {
        id: messageListView
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
        
        model: RoomService.messageModel
        Component.onCompleted: positionViewAtBeginning()

        QQC2.RoundButton {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.largeSpacing + messageListView.headerItem.height
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            id: goMarkAsReadFab

            visible: !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    goToLastMessage();
                    currentRoom.markAllMessagesAsRead();
                }
                icon.name: "go-down"
            }

            QQC2.ToolTip {
                text: i18n("Jump to latest message")
            }
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: ChatBoxHelper.attachmentPath = drop.urls[0]
        }

        QQC2.Pane {
            visible: dropAreaFile.containsDrag
            anchors {
                fill: parent
                margins: Kirigami.Units.gridUnit
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                text: i18n("Drag items here to share them")
            }
        }

        QQC2.Popup {
            anchors.centerIn: parent

            id: attachDialog

            padding: 16

            contentItem: RowLayout {
                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    icon.name: 'mail-attachment'

                    text: i18n("Choose local file")

                    onClicked: {
                        attachDialog.close()

                        var fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            ChatBoxHelper.attachmentPath = path;
                        })

                        fileDialog.open()
                    }
                }

                Kirigami.Separator {}

                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    padding: 16

                    icon.name: 'insert-image'
                    text: i18n("Clipboard image")
                    onClicked: {
                        const localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                        if (!Clipboard.saveImage(localPath)) {
                            return;
                        }
                        ChatBoxHelper.attachmentPath = localPath;
                        attachDialog.close();
                    }
                }
            }
        }
        Component {
            id: openFileDialog
            FileDialog {}
        }
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
    footer: ChatBox {
        id: chatBox
        visible: !invitation.visible && !(messageListView.count === 0 && !currentRoom.allHistoryLoaded)
        onMessageSent: {
            if (!messageListView.atYEnd) {
                goToLastMessage();
            }
        }
        onEditLastUserMessage: {
            const targetMessage = messageEventModel.getLastLocalUserMessageEventId();
            if (targetMessage) {
                ChatBoxHelper.edit(targetMessage["body"], targetMessage["body"], targetMessage["event_id"]);
                chatBox.focusInputField();
            }
        }
        onReplyPreviousUserMessage: {
            const replyResponse = messageEventModel.getLatestMessageFromIndex(0);
            if (replyResponse && replyResponse["event_id"]) {
                ChatBoxHelper.replyToMessage(replyResponse["event_id"], replyResponse["event"], replyResponse["sender_id"]);
            }
        }
    }

    function goToLastMessage() {
        // scroll to the very end, i.e to messageListView.YEnd
        messageListView.positionViewAtIndex(0, ListView.End)
    }
}
