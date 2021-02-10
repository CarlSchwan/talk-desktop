import QtQuick 2.0
import QtGraphicalEffects 1.0
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
            onClicked: {
                var separator = "";
                if(textField.length > 0 && textField.text.substring(textField.length -1) !== " ") {
                    separator = " ";
                }

                textField.text = textField.text + separator + "@" + userId;
                pageStack.navigateBack(PageStackAction.Animated);
                console.log(avatar.source)
                console.log(avatar.accountId)
                console.log(avatar.userId)
            }

            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingMedium
                }
                Image {
                    id: presenceUnderlay
                    source: "image://theme/icon-s-clear-opaque-background"
                    sourceSize.height: avatar.height + Theme.paddingSmall
                    sourceSize.width: avatar.width + Theme.paddingSmall
                    opacity: isOnline ? 0.8 : 0.25
                    BusyIndicator {
                        size: presenceUnderlay.size - Theme.paddingSmall * 2
                        anchors.centerIn: presenceUnderlay
                        running: avatar.status != Image.Ready
                    }

                    ColorOverlay {
                        anchors.fill: presenceUnderlay
                        source: presenceUnderlay
                        opacity: isOnline ? 0.8 : 0.25
                        color: {
                            if ( isOnline ) {
                                switch (presence) {
                                case 2: // away
                                    return Theme.rgba(Theme.presenceColor(Theme.PresenceAway), 1.0);
                                case 3: // busy
                                    return Theme.rgba(Theme.presenceColor(Theme.PresenceBusy), 1.0);
                                default: // seems to be online
                                    return Theme.rgba(Theme.presenceColor(Theme.PresenceAvailable), 1.0);
                                }
                            } else {
                                return Theme.rgba(Theme.presenceColor(Theme.PresenceOffline));
                            }
                        }
                    }
                }
                Avatar {
                    anchors.centerIn: presenceUnderlay
                    id: avatar
                    account: accountId
                    user: userId
                    BusyIndicator {
                        size: avatar.size - Theme.paddingSmall * 2
                        anchors.centerIn: avatar
                        running: !participants.visible
                    }
                }
                Column {
                    id: nameColumn
                    spacing: Theme.paddingSmall
                    anchors {
                        left: presenceUnderlay.right
                        margins: Theme.paddingSmall
                        verticalCenter:  presenceUnderlay.verticalCenter
                    }
                    Label {
                        id: type
                        text: {
                            // https://github.com/nextcloud/spreed/blob/master/lib/Participant.php
                            if(participantType === 1 || participantType === 2) {
                                return qsTr("Moderator");
                            } else if(participantType === 4) {
                                return qsTr("Guest");
                            }
                            return "";
                        }
                        visible: type.text != ''
                        font.weight: Font.Light
                        font.pixelSize: Theme.Theme.fontSizeExtraSmall
                        color: Theme.secondaryHighlightColor
                        anchors.bottom: name.top
                    }
                    Label {
                        id: name
                        text: displayName
                        anchors.verticalCenter: nameColumn.verticalCenter
                        // presenceAvailable is too bright/ugly for styling the whole text:
                        color: isOnline ? Theme.primaryColor : Theme.presenceColor(Theme.PresenceOffline)
                    }
                    Label {
                        id: statusmessage
                        text: {
                            if (presence === 2) {
                                return "... is away";
                                //TODO: implement awayMessage reading
                                //return awayMessage;
                            } else if (presence === 3) {
                                return "do not disturb";
                            } else {
                                return "";
                            }
                        }
                        visible: statusmessage.text != ''
                        font.italic: true
                        font.pixelSize: Theme.Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        anchors.top: name.bottom
                    }
                }
            }
        }

        model: participantService

        VerticalScrollDecorator {}
    }
}
