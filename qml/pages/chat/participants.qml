import QtQuick 2.0
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

        header: PageHeader {
            id: header
            title: qsTr("Participants")
        }

        delegate: BackgroundItem {
            height: Math.max(avatarWrapper.height, userInfoWrapper.height) + Theme.paddingMedium
            width: parent.width - Theme.horizontalPageMargin * 2
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
                anchors.verticalCenter: parent.verticalCenter

                Avatar {
                    anchors.centerIn: parent
                    id: avatar
                    account: accountId
                    user: userId
                    size: Theme.itemSizeSmall
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
                height: type.height + name.height + statusmessage.height
                width: parent.width - avatarWrapper.width - Theme.paddingMedium

                Label {
                    id: type
                    height: visible ? implicitHeight : 0
                    width: parent.width
                    visible: type.text != ''

                    text: {
                        // https://github.com/nextcloud/spreed/blob/master/lib/Participant.php
                        if(participantType === 1 || participantType === 2) {
                            return qsTr("moderator");
                        } else if(participantType === 4) {
                            return qsTr("guest");
                        }
                        return "";
                    }
                    font.weight: Font.Light
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    id: name
                    anchors.top: type.bottom
                    width: parent.width

                    text: displayName
                    color: isOnline ? Theme.primaryColor : Theme.presenceColor(Theme.PresenceOffline)
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    id: statusmessage
                    anchors.top: name.bottom
                    height: visible ? implicitHeight : 0
                    width: parent.width
                    visible: text != ''

                    text: {
                        if (presenceStatus === PresenceStatus.Away) {
                            return "... is away";
                            //TODO: implement awayMessage reading
                            //return awayMessage;
                        } else if (presenceStatus === PresenceStatus.DND) {
                            return "do not disturb";
                        } else {
                            return "";
                        }
                    }
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
            }
        }

        model: participantService

        VerticalScrollDecorator {}
    }
}
