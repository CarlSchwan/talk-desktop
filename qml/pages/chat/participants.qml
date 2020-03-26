import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

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
            }

            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingMedium
                }
                Image {
                    id: avatar
                    source: "image://avatar/" + accountId + "/" + userId + "/"
                    height: Theme.itemSizeExtraSmall
                    width: Theme.itemSizeExtraSmall
                }
                Label {
                    id: name
                    text: displayName
                    font.weight: isOnline ? Font.Bold : Font.Normal
                    leftPadding: avatar.visible ? Theme.paddingMedium : 0
                    verticalAlignment: "AlignVCenter"
                    height: Theme.itemSizeExtraSmall
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
                    font.italic: true
                    leftPadding: Theme.paddingMedium
                    font.weight: Font.Light
                    verticalAlignment: "AlignVCenter"
                    height: Theme.itemSizeExtraSmall
                }
            }
        }

        model: participantService

        VerticalScrollDecorator {}
    }
}
