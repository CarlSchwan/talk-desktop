// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.9-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0
import '../../components/'
import '../../ChatBox'

Kirigami.ScrollablePage {
    id: room

    Item {
        id: hoverActions
        property var event: null
        property bool showEdit: false // Not implemented yet on the server
        property var bubble: null
        property var hovered: bubble && bubble.hovered
        property var visibleDelayed: (hovered || hoverHandler.hovered) && !Kirigami.Settings.isMobile
        onVisibleDelayedChanged: if (visibleDelayed) {
            visible = true;
        } else {
            // HACK: delay disapearing by 200ms, otherwise this can create some glitches
            // See https://invent.kde.org/network/neochat/-/issues/333
            hoverActionsTimer.restart();
        }
        Timer {
            id: hoverActionsTimer
            interval: 200
            onTriggered: hoverActions.visible = hoverActions.visibleDelayed;
        }
        x: bubble ? (bubble.x + Kirigami.Units.largeSpacing + Math.max(bubble.width - childWidth, 0) - (Config.compactLayout ? Kirigami.Units.gridUnit * 3 : 0)) : 0
        y: bubble ? bubble.mapToItem(parent, 0, 0).y - hoverActions.childHeight + Kirigami.Units.smallSpacing: 0;
        visible: false

        property var updateFunction

        property alias childWidth: hoverActionsRow.width
        property alias childHeight: hoverActionsRow.height

        RowLayout {
            id: hoverActionsRow
            z: 4
            spacing: 0
            HoverHandler {
                id: hoverHandler
                margin: Kirigami.Units.smallSpacing
            }

            QQC2.Button {
                QQC2.ToolTip.text: i18n("Download")
                QQC2.ToolTip.visible: hovered
                icon.name: "edit-download"
                visible: hoverActions.event.eventType === MessageEventModel.SingleLinkImageMessage
                onClicked: {
                    fileDialog.open()
                    fileDialog.currentFile = fileDialog.folder + '/' + hoverActions.event.fileName
                }

                FileDialog {
                    id: fileDialog
                    title: i18n("Please choose a file")
                    folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
                    onAccepted: DownloadService.getFile(hoverActions.event.fileUrl, fileDialog.file, RoomListModel.messageModel.accountId)
                    fileMode: FileDialog.SaveFile
                }
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Reply")
                QQC2.ToolTip.visible: hovered
                icon.name: "mail-replied-symbolic"
                onClicked: {
                    ChatBoxHelper.replyToMessage(hoverActions.event.eventId, hoverActions.event.message, hoverActions.event.author);
                    chatBox.focusInputField();
                }
            }
        }
    }

    ListView {
        id: messageListView
        delegate: DelegateChooser {
            id: timelineDelegateChooser
            role: "eventType"

            property bool delegateLoaded: true
            ListView.onPooled: delegateLoaded = false
            ListView.onReused: delegateLoaded = true

            DelegateChoice {
                roleValue: MessageEventModel.RegularTextMessage
                delegate: TimelineContainer {
                    id: messageContainer
                    width: ListView.view.width
                    hoverComponent: hoverActions
                    isLoaded: true
                    innerObject: TextDelegate {
                        Layout.maximumWidth: messageContainer.bubbleMaxWidth
                        onRequestOpenMessageContext: openMessageContext(model, parent.selectedText)
                    }
                }
            }
            DelegateChoice {
                roleValue: MessageEventModel.SingleLinkImageMessage
                delegate: TimelineContainer {
                    id: imageContainer
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    width: messageListView.width
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: ImageDelegate {
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 15
                        Layout.maximumWidth: imageContainer.bubbleMaxWidth
                        Layout.preferredHeight: info.h / info.w * width
                        Layout.maximumHeight: Kirigami.Units.gridUnit * 20
                        //TapHandler {
                        //    acceptedButtons: Qt.RightButton
                        //    onTapped: openFileContext(model, parent)
                        //}
                        //TapHandler {
                        //    acceptedButtons: Qt.LeftButton
                        //    onLongPressed: openFileContext(model, parent)
                        //    onTapped: {
                        //        fullScreenImage.createObject(parent, {
                        //            filename: eventId,
                        //            localPath: currentRoom.urlToDownload(eventId),
                        //            blurhash: model.content.info["xyz.amorgan.blurhash"],
                        //            imageWidth: content.info.w,
                        //            imageHeight: content.info.h
                        //        }).showFullScreen();
                        //    }
                        //}
                    }
                }
            }
        }

        model: RoomListModel.messageModel
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
    background: Item {}
}
