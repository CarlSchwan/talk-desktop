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
                console.log(avatar.source)
                console.log(avatar.accountId)
                console.log(avatar.userId)
            }

            SilicaItem {
                id: avatarWrapper
                width: presenceUnderlay.implicitWidth
                height: presenceUnderlay.implicitHeight

                Image {
                    id: presenceUnderlay
                    source: {
                            if (presenceStatus != 0 ) {
                                if (presenceStatus === PresenceStatus.Online) {
                                    return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceAvailable), 1.0);
                                } else if (presenceStatus === PresenceStatus.Away) {
                                    return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceAway), 0.8);
                                } else if (presenceStatus === PresenceStatus.DND) {
                                    return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceBusy), 1.0);
                                } else if (presenceStatus === PresenceStatus.Invisible) {
                                    return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceOffline), 0.8);
                                } else { // seems to be online, unknown presence
                                    return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceAvailable), 1.0);
                                }
                            } else {
                                return "image://theme/icon-s-clear-opaque-background?" + Theme.rgba(Theme.presenceColor(Theme.PresenceOffline), 1.0);
                            }
                    }
                    sourceSize.height: avatar.height + Theme.paddingSmall
                    sourceSize.width: avatar.width + Theme.paddingSmall
                    opacity: presenceStatus != 0 ? 0.8 : 0.25
                    BusyIndicator {
                        size: presenceUnderlay.implicitWidth - Theme.paddingSmall * 2
                        anchors.centerIn: presenceUnderlay
                        running: avatar.status != Image.Ready
                    }

                }
                Avatar {
                    anchors.centerIn: presenceUnderlay
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
            }

            SilicaItem {
                id: userInfoWrapper
                anchors {
                    left: avatarWrapper.right
                    margins: Theme.paddingSmall
                    verticalCenter:  avatarWrapper.verticalCenter
                }
                height: type.implicitHeight + name.implicitHeight + statusmessage.implicitHeight

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
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.secondaryHighlightColor
                    anchors.bottom: name.top
                }

                Label {
                    id: name
                    text: displayName
                    anchors.verticalCenter: userInfoWrapper.verticalCenter
                    // presenceAvailable is too bright/ugly for styling the whole text:
                    color: presenceStatus != 0 ? Theme.primaryColor : Theme.presenceColor(Theme.PresenceOffline)
                }
                Label {
                    id: statusmessage
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
                    visible: text != ''
                    font.italic: true
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    anchors.top: name.bottom
                }
            }
        }

        model: participantService

        VerticalScrollDecorator {}
    }
}
