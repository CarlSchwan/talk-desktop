// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.1 as Platform
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0
import '../components'

Item {
    id: root
    property alias inputFieldText: chatBar.inputFieldText

    signal messageSent()
    signal editLastUserMessage()
    signal replyPreviousUserMessage()

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    implicitWidth: {
        let w = 0
        for(let i = 0; i < visibleChildren.length; ++i) {
            w = Math.max(w, Math.ceil(visibleChildren[i].implicitWidth))
        }
        return w
    }
    implicitHeight: {
        let h = 0
        for(let i = 0; i < visibleChildren.length; ++i) {
            h += Math.ceil(visibleChildren[i].implicitHeight)
        }
        return h
    }

    // For some reason, this is needed to make the height animation work even though
    // it used to work and height should be directly affected by implicitHeight
    height: implicitHeight

    Behavior on height {
        NumberAnimation {
            property: "height"
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }

    Kirigami.Separator {
        id: connectionPaneSeparator
        visible: connectionPane.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: connectionPane.top
        z: 1
    }

    Kirigami.Separator {
        id: emojiPickerLoaderSeparator
        visible: emojiPickerLoader.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: emojiPickerLoader.top
        z: 1
    }

    Loader {
        id: emojiPickerLoader
        active: visible
        visible: chatBar.emojiPaneOpened
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: replySeparator.top
        sourceComponent: EmojiPicker{
            textArea: chatBar.textField
            onChosen: addText(emoji)
        }
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }

    }

    Kirigami.Separator {
        id: replySeparator
        visible: replyPane.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: replyPane.top
    }


    ReplyPane {
        id: replyPane
        visible: ChatBoxHelper.isReplying || ChatBoxHelper.isEditing
        user: ChatBoxHelper.replyUser
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: attachmentSeparator.top
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
        onReplyCancelled: {
            root.focusInputField()
        }
    }

    Kirigami.Separator {
        id: attachmentSeparator
        visible: attachmentPane.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: attachmentPane.top
    }

    AttachmentPane {
        id: attachmentPane
        visible: ChatBoxHelper.hasAttachment
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: chatBarSeparator.top
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    Kirigami.Separator {
        id: chatBarSeparator
        visible: chatBar.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: chatBar.top
    }

    ChatBar {
        id: chatBar
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: parent.bottom

        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }

        onCloseAllTriggered: closeAll()
        onMessageSent: {
            closeAll()
            root.messageSent();
        }
        onEditLastUserMessage: {
            root.editLastUserMessage();
        }
        onReplyPreviousUserMessage: {
            root.replyPreviousUserMessage();
        }
    }

    function addText(text) {
        root.inputFieldText = inputFieldText + text
    }

    function insertText(str) {
        root.inputFieldText = inputFieldText.substr(0, inputField.cursorPosition) + str + inputFieldText.substr(inputField.cursorPosition)
    }

    function focusInputField() {
        chatBar.inputFieldForceActiveFocusTriggered()
    }

    Connections {
        target: RoomListModel.currentNameChanged

        function onCurrentRoomChanged() {
            chatBar.userAutocompleted = {};
        }
    }

    Connections {
        target: ChatBoxHelper

        function onShouldClearText() {
            root.inputFieldText = "";
        }

        function onEditing(editContent, editFormatedContent) {
            // Set the input field in edit mode
            root.inputFieldText = editContent;

            // clean autocompletion list
            chatBar.userAutocompleted = {};

            // Fill autocompletion list with values extracted from message.
            // We can't just iterate on every user in the list and try to
            // find matching display name since some users have display name
            // matching frequent words and this will marks too many words as
            // mentions.
            const regex = /<a href=\"https:\/\/matrix.to\/#\/(@[a-zA-Z09]*:[a-zA-Z09.]*)\">([^<]*)<\/a>/g;

            let match;
            while ((match = regex.exec(editFormatedContent.toString())) !== null) {
                chatBar.userAutocompleted[match[2]] = match[1];
            }
            chatBox.forceActiveFocus();
        }
    }

    function closeAll() {
        ChatBoxHelper.clear();
        chatBar.emojiPaneOpened = false;
    }
}
