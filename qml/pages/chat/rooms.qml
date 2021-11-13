import QtQuick 2.5
import QtQml 2.2
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0
import "../../components"

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
                    width: parent.width

                    Row {
                        id: nameRow
                        width: parent.width - unreadCounterItem.width
                        spacing: Theme.paddingMedium

                        Rectangle {
                            id: accountIndicator
                            width: Theme.paddingSmall / 3
                            height: roomName.paintedHeight + lastMessage.paintedHeight
                            color: primaryColor
                        }

                        Loader {
                            id: conversationLogo
                            width: Theme.iconSizeMedium
                            asynchronous: false

                            // bind Avatar properties
                            property int _account: accountId
                            property string _user: conversationName
                            property int _size: Theme.iconSizeMedium
                            // bind ConversationIcon properties
                            property int _conversationType: conversationType

                            source: {
                                switch (conversationType) {
                                case ConversationType.OneToOne:
                                    return "../../components/Avatar.qml"
                                default:

                                    return "../../components/ConversationIcon.qml"
                                }
                            }
                        }


                        Column {
                            width: parent.width - accountIndicator.width - conversationLogo.width - unreadCounterItem.width

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
                        width: unreadCounter.text == "" ? Theme.horizontalPageMargin : Theme.iconSizeMedium
                        height: unreadCounter.text == "" ? 0 : parent.height
                        visible: unreadCounter.text != ""

                        Rectangle {
                            width: Theme.iconSizeMedium * 0.8
                            height: Theme.iconSizeMedium * 0.8
                            anchors.centerIn: unreadCounterItem
                            border.color: unreadCounter.color
                            border.width: 2
                            radius: Theme.iconSizeMedium
                            color: unreadMention ? Theme.secondaryHighlightColor : "transparent"
                            antialiasing: true
                        }

                        Label {
                            id: unreadCounter
                            text: unreadMessages === 0 ? ""
                                    : unreadMessages > 99 ? "99+"
                                    : unreadMessages
                            color: Theme.primaryColor
                            anchors.centerIn: parent
                            font.pixelSize: Theme.fontSizeMedium
                            font.weight: Font.Bold
                            fontSizeMode: Text.Fit
                            horizontalAlignment: Qt.AlignHCenter
                            width: parent.visible ? parent.width * 0.7 : 0
                            height: parent.width * 0.7
                            antialiasing: true
                            verticalAlignment: Qt.AlignVCenter
                        }
                    }
                }
            }

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

        model:  RoomListModel {
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
