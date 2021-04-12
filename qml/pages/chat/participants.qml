import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0
import "../../components/"

Page {
    id: participants
    allowedOrientations: Orientation.All

    property string token;
    property int accountId;
    property Item textField;

    readonly property string pageName: "Participants";

    ParticipantService {
        id: participantService
    }

    Timer {
        id: participantsPolling
        interval: participants.visible ? 5000 : 30000;
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: {
            participantService.pullParticipants(participants.token, participants.accountId)
        }
    }

    SilicaListView {
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - Theme.horizontalPageMargin * 3 // 3 instead of 2 because of x: of the delegate
        spacing: Theme.paddingLarge

        add:       Transition { NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 1000 } }
        remove:    Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 1000 } }

        header: PageHeader {
            id: header
            title: qsTr("Participants")
        }

        delegate: BackgroundItem {
            contentHeight: Math.max(avatarWrapper.height, userInfoWrapper.height, userInfoWrapper.minHeight) + Theme.paddingMedium
            width: ListView.view.width
            x: Theme.horizontalPageMargin

            onClicked: {
                var separator = "";
                if(textField.length > 0 && textField.text.substring(textField.length -1) !== " ") {
                    separator = " ";
                }

                textField.text = textField.text + separator + "@" + userId;
                pageStack.navigateBack(PageStackAction.Animated);
            }

            SilicaItem {
                id: avatarWrapper
                width: avatar.implicitWidth + userStatusIcon.width
                height: avatar.implicitHeight

                // hack: assign once instead of binding.
                // when userInfoWrapper increases height, this would jump around with a bind
                // as long as userInfoWrapper.name binds to parent.verticalCenter this should keep the avatar center-aligned with the name.
                anchors.verticalCenter: { return parent.verticalCenter }

                Avatar {
                    anchors.centerIn: parent
                    id: avatar
                    account: accountId
                    user: userId
                    size: Theme.itemSizeSmall
                    opacity: isOnline ? 1 : 0.4
                    Behavior on opacity { FadeAnimator { duration: 1000 } }
                    BusyIndicator {
                        size: avatar.size - Theme.paddingSmall * 2
                        anchors.centerIn: avatar
                        running: !participants.visible
                    }
                }
                Loader {
                    id: userStatusIcon
                    width: avatar.size * 0.4
                    height: avatar.size * 0.4

                    y: avatar.y + avatar.height + Theme.paddingSmall - height
                    x: avatar.x + avatar.width + Theme.paddingSmall - width

                    opacity: visible ? 0.8 : 0
                    Behavior on opacity { FadeAnimator { duration: 1000 } }

                    source: {
                        switch (presenceStatus) {
                        case PresenceStatus.Online:
                            return "../../components/UserStatus/NcIconOnline.qml"
                        case PresenceStatus.Away:
                            return "../../components/UserStatus/NcIconAway.qml"
                        case PresenceStatus.DND:
                            return "../../components/UserStatus/NcIconDnd.qml"
                        default:
                            // when none is set, nothing should be shown
                            return ""
                        }
                    }
                }
            }

            SilicaItem {
                id: userInfoWrapper
                anchors {
                    left: avatarWrapper.right
                    leftMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                height: type.height + name.height + statusMessageLabel.height
                width: parent.width - avatarWrapper.width - Theme.paddingMedium
                property int minHeight: type.implicitHeight + name.implicitHeight + statusMessageLabel.implicitHeight

                function checkFolding() {
                    unfoldButton.visible = name.isTooLong || statusMessageLabel.isTooLong
                }

                Label {
                    id: type
                    anchors.bottom: name.top
                    height: visible ? implicitHeight : 0
                    width: parent.width

                    text: {
                        // https://github.com/nextcloud/spreed/blob/master/lib/Participant.php
                        if (isModerator) {
                            return qsTr("moderator");
                        }
                        return "";
                    }
                    font.weight: Font.Light
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    property bool isTooLong: false

                    id: name
                    anchors.verticalCenter: parent.verticalCenter
                    width: unfoldButton.visible ? parent.width - unfoldButton.width : parent.width

                    text: displayName
                    color: isOnline ? Theme.primaryColor : Theme.presenceColor(Theme.PresenceOffline)
                    truncationMode: TruncationMode.Fade

                    onTextChanged: {
                        textMetrics.font = font
                        textMetrics.font.pixelSize = Theme.fontSizeMedium // a smaller font is used otherwise 🤷
                        textMetrics.text = text
                        isTooLong = textMetrics.elidedText !== text
                    }

                    onIsTooLongChanged: {
                        parent.checkFolding()
                    }
                }
                Label {
                    property bool isTooLong: false

                    id: statusMessageLabel
                    anchors.top: name.bottom
                    height: visible ? implicitHeight : 0
                    width: unfoldButton.visible ? parent.width - unfoldButton.width : parent.width

                    text: {
                        // user provided message or icon
                        var newText = "";
                        if (statusIcon != "" || statusMessage != "") {
                            newText = (statusIcon + " " + statusMessage).trim();
                        }
                        // fallback: depending on announcable presence
                        else if (presenceStatus === PresenceStatus.Away) {
                            newText = qsTr("away");
                        } else if (presenceStatus === PresenceStatus.DND) {
                            newText = qsTr("do not disturb");
                        }

                        return newText
                    }
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.secondaryColor
                    truncationMode: TruncationMode.Fade

                    onTextChanged: {
                        textMetrics.text = text
                        textMetrics.font = font
                        isTooLong = textMetrics.elidedText !== text
                    }

                    onIsTooLongChanged: {
                        parent.checkFolding()
                    }
                }

                TextMetrics {
                    id: textMetrics
                    elideWidth: userInfoWrapper.width
                    elide: Qt.ElideRight
                }

                IconButton {
                    id: unfoldButton
                    icon.source: "image://theme/icon-m-change-type?" + Theme.secondaryColor
                    width: Theme.iconSizeSmall
                    anchors.left: name.right
                    anchors.top: name.top
                    visible: false
                    Behavior on rotation { PropertyAnimation { } }

                    onClicked: {
                        if(rotation === 0) {
                            name.maximumLineCount = 5
                            name.wrapMode = Text.Wrap
                            statusMessageLabel.maximumLineCount = 5
                            statusMessageLabel.wrapMode = Text.Wrap
                            rotation = 180
                        } else {
                            name.maximumLineCount = 1
                            name.wrapMode = Text.NoWrap
                            statusMessageLabel.maximumLineCount = 1
                            statusMessageLabel.wrapMode = Text.NoWrap
                            rotation = 0
                        }
                    }
                }


            }
        }

        model: participantService

        VerticalScrollDecorator {}
    }
}
