// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0
import harbour.nextcloud.talk 1.0

Kirigami.OverlayDrawer {
    id: roomDrawer
    property string token
    property int accountId

    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    contentItem: Loader {
        active: roomDrawer.drawerOpen
        sourceComponent: ColumnLayout {
            id: columnLayout
            anchors.fill: parent
            spacing: 0

            Timer {
                id: participantsPolling
                interval: roomDrawer.drawerOpen ? 5000 : 30000;
                repeat: true
                running: true
                triggeredOnStart: true
                onTriggered: RoomService.participants.pullParticipants()
            }

            Kirigami.AbstractApplicationHeader {
                Layout.fillWidth: true
                topPadding: Kirigami.Units.smallSpacing / 2;
                bottomPadding: Kirigami.Units.smallSpacing / 2;
                rightPadding: Kirigami.Units.smallSpacing
                leftPadding: Kirigami.Units.smallSpacing

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    ToolButton {
                        icon.name: "list-add-user"
                        text: i18n("Invite")
                        onClicked: {
                            //applicationWindow().pageStack.layers.push("qrc:/imports/NeoChat/Page/InviteUserPage.qml", {"room": room})
                            //roomDrawer.close();
                        }
                    }
                    Item {
                        // HACK otherwise rating item is not right aligned
                        Layout.fillWidth: true
                    }

                    ToolButton {
                        Layout.alignment: Qt.AlignRight
                        icon.name: RoomService.isFavourite ? "rating" : "rating-unrated"
                        checkable: true
                        enabled: RoomService.isLoaded
                        checked: RoomService.currentIsFavourite
                        onClicked: RoomService.currentIsFavourite != RoomService.currentIsFavourite
                        ToolTip {
                            text: RoomService.currentIsFavourite ? i18n("Remove room from favorites") : i18n("Make room favorite")
                        }
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignRight
                        icon.name: 'settings-configure'
                        onClicked: {
                        }

                        ToolTip {
                            text: i18n("Room settings")
                        }
                    }
                }
            }

            Control {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing
                contentItem: ColumnLayout {
                    id: infoLayout
                    Layout.fillWidth: true
                    Kirigami.Heading {
                        text: i18n("Room information")
                        level: 3
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.largeSpacing

                        spacing: Kirigami.Units.largeSpacing

                        Kirigami.Avatar {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 3.5

                            name: RoomService.currentName
                            source: RoomService.currentAvatarUrl
                            visible: RoomService.currentAvatarUrl.length !== 0
                        }

                        Kirigami.Heading {
                            Layout.maximumWidth: Kirigami.Units.gridUnit * 9
                            Layout.fillWidth: true
                            font.bold: true
                            wrapMode: Label.Wrap
                            text: RoomService.currentName
                        }
                    }

                    TextEdit {
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 13
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 13
                        Layout.fillWidth: true
                        text: RoomService.currentDescription ? RoomService.currentDescription.replace(replaceLinks, "<a href=\"$1\">$1</a>") : i18n("No Topic")
                        readonly property var replaceLinks: /\(https:\/\/[^ ]*\)/
                        textFormat: TextEdit.MarkdownText
                        wrapMode: Text.WordWrap
                        selectByMouse: true
                        color: Kirigami.Theme.textColor
                        onLinkActivated: Qt.openUrlExternally(link)
                        readOnly: true
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                        }
                    }
                }
            }

            Kirigami.ListSectionHeader {
                label: i18n("Members")
                activeFocusOnTab: false
                Label {
                    Layout.alignment: Qt.AlignRight
                    text: room ? i18np("%1 Member", "%1 Members", room.totalMemberCount) : i18n("No Member Count")
                }
            }

            Pane {
                padding: Kirigami.Units.smallSpacing
                implicitWidth: parent.width
                z: 2
                contentItem: Kirigami.SearchField {
                    id: userListSearchField
                    onAccepted: sortedMessageEventModel.filterString = text;
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: userListView
                    clip: true
                    headerPositioning: ListView.OverlayHeader
                    boundsBehavior: Flickable.DragOverBounds
                    activeFocusOnTab: true

                    model: KSortFilterProxyModel {
                        id: sortedMessageEventModel

                        sourceModel: RoomService.participants

                        sortRole: "isOnline"
                        filterRole: "displayName"
                    }

                    delegate: Kirigami.AbstractListItem {
                        width: userListView.width
                        implicitHeight: Kirigami.Units.gridUnit * 2
                        z: 1

                        contentItem: RowLayout {
                            Kirigami.Avatar {
                                Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                visible: Config.showAvatarInTimeline
                                sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                source: model.avatar
                                name: displayName
                            }

                            Label {
                                Layout.fillWidth: true

                                text: displayName
                                textFormat: Text.PlainText
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }

                            Label {
                                visible: isModerator

                                text: i18n("Moderator")
                                color: Kirigami.Theme.disabledTextColor
                                font.pixelSize: 12
                                textFormat: Text.PlainText
                                wrapMode: Text.NoWrap
                            }
                        }
                    }
                }
            }
        }
    }
}
