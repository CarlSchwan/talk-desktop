import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: rooms
    allowedOrientations: Orientation.All

    property string lastUpdate: ""

    Timer {
        id: roomPolling
        interval: rooms.visible ? 5000 : 30000;
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: {
            roomService.loadRooms()
        }
    }

    SilicaListView {
        PullDownMenu {
            id: pulley
             MenuItem {
                 text: qsTr("Accounts")
                 onClicked: pageStack.push(Qt.resolvedUrl("../Accounts.qml"), {})
             }
             MenuItem {
                 text: qsTr("Last update: ") + lastUpdate
                 enabled: false
             }
         }

        TextMetrics {
            id: textMetricsUnread
            text: "99+"
            //font.weight: Font.DemiBold
            font.weight: Font.Bold
            font.pixelSize: Theme.fontSizeMedium
        }

        TextMetrics {
            id: textMetricsUnreadSingleDigit
            text: "9"
            font.weight: Font.Bold
            font.pixelSize: Theme.fontSizeMedium
        }

        id: roomList
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Conversations")
        }

        delegate: BackgroundItem {

            height: roomName.paintedHeight + lastMessage.paintedHeight + Theme.paddingMedium
                Column {
                    id: conversationItem
                    anchors {
                        left: parent.left
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        leftMargin: 0;
                    }

                    Row {
                        width: parent.width - Theme.horizontalPageMargin
                        Row {
                            id: nameRow
                            width: parent.width
                            spacing: Theme.paddingMedium

                            Rectangle {
                                id: accountIndicator
                                width: Theme.paddingSmall / 3
                                height: roomName.paintedHeight + lastMessage.paintedHeight
                                color: primaryColor
                            }

                            Image {
                                id: conversationLogo
                                //TODO: image from model
                                //source: "image://theme/icon-m-users"
                                source: {
                                    switch (conversatiorType) {
                                        case 3:
                                            return "image://theme/icon-m-region"

                                        case 2:
                                        default:
                                            return "image://theme/icon-m-users"
                                    }
                                }

                                width: Theme.iconSizeMedium
                                fillMode: Image.PreserveAspectFit
                                height: roomName.paintedHeight + lastMessage.paintedHeight
                                verticalAlignment: Qt.AlignVCenter
                            }

                            }

                            Column {
                                width: parent.width - accountIndicator.width - conversationLogo.width - unreadCounter.width
                                Label {
                                    id: roomName
                                    width: parent.width
                                    text: name
                                    font.pixelSize: Theme.fontSizeMedium;
                                    font.weight: Font.DemiBold
                                    maximumLineCount: 1
                                    truncationMode: TruncationMode.Fade
                                }

                                Row {
                                    width: parent.width

                                    Label {
                                        id: lastMessageAuthorName
                                        text: lastMessageIsSystemMessage ? "" : lastMessageAuthor + " "
                                        font.pixelSize: Theme.fontSizeExtraSmall
                                        font.weight: Font.Thin
                                        maximumLineCount: 1
                                        truncationMode: TruncationMode.Fade
                                        color: Theme.secondaryColor
                                    }

                                    Label {
                                        width: parent.width - lastMessageAuthorName.width
                                        id: lastMessage
                                        text: lastMessageText
                                        font.pixelSize: Theme.fontSizeExtraSmall
                                        font.weight: Font.Thin
                                        maximumLineCount: 1
                                        truncationMode: TruncationMode.Fade
                                    }

                                }
                            }
                        }

                        Item {
                            id: unreadCounterItem
                            width: textMetricsUnread.width
                            height: parent.height
                            visible: unreadCounter.text != ""

                            Rectangle {
                                width: unreadCounter.paintedWidth * 2
                                height: parent.height * 0.7
                                border.color: unreadCounter.color
                                border.width: 1
                                radius: 75
                                color: unreadMention ? Theme.secondaryHighlightColor : "transparent"
                                y: parent.height * 0.15
                                antialiasing: true
                                x: {
                                    if(unreadCounter.text == "") {
                                        return 0;
                                    }
                                    var factor = unreadCounter.text.length - 1;

                                    //this.width = textMetricsUnreadSingleDigit.width * (unreadCounter.text.length + 1.6 + factor/10 * 2) // large font size
                                    this.width = textMetricsUnreadSingleDigit.width * (unreadCounter.text.length + 1.2 + factor/10 * 2)
                                    return 0 - textMetricsUnreadSingleDigit.width * factor * 1.2
                                }
                            }

                            Label {
                                id: unreadCounter
                                text: unreadMessages === 0 ? ""
                                        : unreadMessages > 99 ? "99+"
                                        : unreadMessages
                                color: Theme.primaryColor
                                font.pixelSize: textMetricsUnread.font.pixelSize;
                                font.weight: textMetricsUnread.font.weight
                                horizontalAlignment: Qt.AlignRight
                                width: parent.visible ? textMetricsUnread.width : 0
                                height: parent.height
                                rightPadding: Theme.horizontalPageMargin
                                antialiasing: true
                                verticalAlignment: Qt.AlignVCenter
                            }
                        }
                    }
                }
 //           }

            onClicked: {
                pageStack.push(Qt.resolvedUrl("./room.qml"),
                               {
                                   "token": token,
                                   "roomName": name,
                                   "accountId": accountId,
                                   "accountUserId": accountUserId,
                               }
                );
            }
        }

        model:  RoomService {
            id: roomService
            onModelAboutToBeReset: pulley.busy = true
            onModelReset: pulley.busy = false
            onDataChanged: {
                lastUpdate = new Date().toLocaleTimeString(undefined, {hour: '2-digit', minute: '2-digit'});
            }
        }

        VerticalScrollDecorator {}
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        running: lastUpdate === ""
        anchors.centerIn: parent
    }
}
