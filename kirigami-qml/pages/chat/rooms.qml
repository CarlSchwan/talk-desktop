// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.15 as Kirigami
import harbour.nextcloud.talk 1.0

Kirigami.ScrollablePage {
    id: page 

    property string lastUpdate: ""

    property bool collapsedMode: Config.roomListPageWidth === applicationWindow().collapsedPageWidth && applicationWindow().shouldUseSidebars

    titleDelegate: collapsedMode ? empty : searchField

    Component {
        id: empty
        Item {}
    }

    Component {
        id: searchField
        Kirigami.SearchField {
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.fillHeight: true
            Layout.fillWidth: true
            onTextChanged: filterModel.filterText = text
            KeyNavigation.tab: roomList
        }
    }

    header: QQC2.ItemDelegate {
        visible: page.collapsedMode
        action: Kirigami.Action {
            id: enterRoomAction
            onTriggered: quickView.item.open();
        }
        topPadding: Kirigami.Units.largeSpacing
        leftPadding: Kirigami.Units.largeSpacing
        rightPadding: Kirigami.Units.largeSpacing
        bottomPadding: Kirigami.Units.largeSpacing
        width: visible ? page.width : 0
        height: visible ? Kirigami.Units.gridUnit * 2 : 0

        Kirigami.Icon {
            anchors.centerIn: parent
            width: 22 * Kirigami.Units.devicePixelRatio
            height: 22 * Kirigami.Units.devicePixelRatio
            source: "search"
        }
        Kirigami.Separator {
            width: parent.width
            anchors.bottom: parent.bottom
        }
    }

    Timer {
        id: roomPolling
        interval: applicationWindow().wideScreen ? 5000 : 30000;
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: RoomService.loadRooms()
    }
    Component.onCompleted: RoomService.loadRooms();

    ListView {
        id: roomList
        activeFocusOnTab: true

        reuseItems: true
        currentIndex: -1 // we don't want any room highlighted by default

        delegate: page.collapsedMode ? collapsedModeListComponent : normalModeListComponent
        
        Kirigami.PlaceholderMessage {
            id: loadingIndicator
            visible: !RoomService.isLoaded
            anchors.centerIn: parent
            text: i18n("Loading...")
            QQC2.BusyIndicator {
                running: loadingIndicator.visible
                Layout.alignment: Qt.AlignHCenter
            }
        }

        Component {
            id: collapsedModeListComponent

            QQC2.ItemDelegate {
                action: Kirigami.Action {
                    id: enterRoomAction
                    onTriggered: {
                        if (applicationWindow().pageStack.depth === 1) {
                            applicationWindow().pageStack.push("qrc:/pages/chat/room.qml", {
                                token: token,
                                roomName: name,
                                accountId: accountId,
                                accountUserId: accountUserId,
                            });
                        } else {
                            const roomPage = applicationWindow().pageStack.get(1);
                            roomPage.token = token;
                            roomPage.roomName = name;
                            roomPage.accountId = accountId;
                            roomPage.accountUserId = accountUserId;
                        }
                    }
                }
                Keys.onEnterPressed: enterRoomAction.trigger()
                Keys.onReturnPressed: enterRoomAction.trigger()
                topPadding: Kirigami.Units.largeSpacing
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                width: ListView.view.width
                height: ListView.view.width

                contentItem: Kirigami.Avatar {
                    source: conversationType === ConversationType.OneToOne ? `image://avatar/${accountId}/${conversationName}/` : ''
                    iconSource: switch (conversationType) {
                    case ConversationType.Public:
                        return 'link';
                    case ConversationType.Group:
                        return 'group';
                    case ConversationType.Changelog:
                        return 'documentinfo';
                    }
                    initialsMode: conversationType === ConversationType.OneToOne ? Kirigami.Avatar.InitialsMode.UseInitials : Kirigami.Avatar.InitialsMode.UseIcon
                    imageMode: conversationType === ConversationType.OneToOne ? Kirigami.Avatar.ImageMode.AdaptiveImageOrInitals : Kirigami.Avatar.ImageMode.AlwaysShowInitials
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }

                QQC2.ToolTip {
                    enabled: text.length !== 0
                    text: name ?? ""
                }
            }
        }

        Component {
            id: normalModeListComponent
            Kirigami.BasicListItem {
                id: roomListItem
                topPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                //highlighted: listView.currentIndex === index
                focus: true
                icon: undefined
                action: Kirigami.Action {
                    id: enterRoomAction
                    onTriggered: {
                        if (applicationWindow().pageStack.depth === 1) {
                            applicationWindow().pageStack.push('qrc:/pages/chat/room.qml', {
                                title: name,
                            });
                        } else {
                            const roomPage = applicationWindow().pageStack.get(1);
                            roomPage.title = name;
                        }
                        RoomService.select(filterModel.mapToSource(filterModel.index(index, 0)).row);
                    }
                }
                Keys.onEnterPressed: enterRoomAction.trigger()
                Keys.onReturnPressed: enterRoomAction.trigger()
                bold: unreadCount > 0
                label: model.name ?? ""
                subtitle: lastMessageIsSystemMessage ? lastMessageText : `${lastMessageAuthor}: ${lastMessageText}`
                subtitleItem.maximumLineCount: 1

                leading: Kirigami.Avatar {
                    source: conversationType === ConversationType.OneToOne ? `image://avatar/${accountId}/${conversationName}/` : ''
                    iconSource: switch (conversationType) {
                    case ConversationType.Public:
                        return 'link';
                    case ConversationType.Group:
                        return 'group';
                    case ConversationType.Changelog:
                        return 'documentinfo';
                    }
                    initialsMode: conversationType === ConversationType.OneToOne ? Kirigami.Avatar.InitialsMode.UseInitials : Kirigami.Avatar.InitialsMode.UseIcon
                    imageMode: conversationType === ConversationType.OneToOne ? Kirigami.Avatar.ImageMode.AdaptiveImageOrInitals : Kirigami.Avatar.ImageMode.AlwaysShowInitials
                    name: model.name || i18n("No Name")
                    implicitWidth: visible ? height : 0
                    visible: Config.showAvatarInTimeline
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }
            }
        }

        model: RoomListFilterModel {
            id: filterModel
            sourceModel: RoomService
        }
    }
}
