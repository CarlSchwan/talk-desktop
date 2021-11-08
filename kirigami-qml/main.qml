// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import harbour.nextcloud.talk 1.0
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.15 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    property int columnWidth: Kirigami.Units.gridUnit * 13

    minimumWidth: Kirigami.Units.gridUnit * 15
    minimumHeight: Kirigami.Units.gridUnit * 20

    wideScreen: width > columnWidth * 5

    contextDrawer: RoomDrawer {
        id: contextDrawer
        edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        modal: !root.wideScreen || !enabled
        onEnabledChanged: drawerOpen = enabled && !modal
        onModalChanged: drawerOpen = !modal
        enabled: RoomService.isLoaded && pageStack.layers.depth < 2 && pageStack.depth < 3
        handleVisible: enabled && pageStack.layers.depth < 2 && pageStack.depth < 3
    }

    readonly property int defaultPageWidth: Kirigami.Units.gridUnit * 17
    readonly property int minPageWidth: Kirigami.Units.gridUnit * 10
    readonly property int collapsedPageWidth: Kirigami.Units.gridUnit * 3 - Kirigami.Units.smallSpacing * 3
    readonly property bool shouldUseSidebars: RoomService.hasOpenRoom && (Config.roomListPageWidth > minPageWidth ? root.width >= Kirigami.Units.gridUnit * 35 : root.width > Kirigami.Units.gridUnit * 27)
    readonly property int pageWidth: {
        if (Config.roomListPageWidth === -1) {
            return defaultPageWidth;
        } else if (Config.roomListPageWidth < minPageWidth) {
            return collapsedPageWidth;
        } else {
            return Config.roomListPageWidth;
        }
    }

    pageStack.defaultColumnWidth: pageWidth
    pageStack.columnView.columnResizeMode: shouldUseSidebars ? Kirigami.ColumnView.FixedColumns : Kirigami.ColumnView.SingleColumn

    globalDrawer: Kirigami.GlobalDrawer {
        property bool hasLayer
        contentItem.implicitWidth: columnWidth
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "settings-configure"
                onTriggered: pushReplaceLayer("qrc:/pages/SettingsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Settings")
                shortcut: StandardKey.Preferences
            }
            /*Kirigami.Action {
                text: i18n("Logout")
                icon.name: "list-remove-user"
                enabled: Controller.accountCount > 0
                onTriggered: Controller.logout(Controller.activeConnection, true)
            },
            Kirigami.Action {
                text: i18n("Quit")
                icon.name: "gtk-quit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }*/
        ]
    }

    MouseArea {
        visible: root.pageStack.wideMode
        z: 500

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        x: root.pageStack.defaultColumnWidth - (width / 2)
        width: Kirigami.Units.devicePixelRatio * 2

        property int _lastX: -1
        enabled: !Kirigami.Settings.isMobile

        cursorShape: !Kirigami.Settings.isMobile ? Qt.SplitHCursor : undefined

        onPressed: _lastX = mouseX
        onReleased: Config.save();

        onPositionChanged: {
            if (_lastX == -1) {
                return;
            }

            if (mouse.x > _lastX) {
                // we moved to the right
                if (Config.roomListPageWidth === root.collapsedPageWidth && root.pageWidth + (mouse.x - _lastX) >= root.minPageWidth) {
                    // Here we get back directly to a more wide mode.
                    Config.roomListPageWidth = root.minPageWidth;
                    if (root.width < Kirigami.Units.gridUnit * 35) {
                        root.width = Kirigami.Units.gridUnit * 35;
                    }
                } else if (Config.roomListPageWidth !== root.collapsedPageWidth) {
                    // Increase page width
                    Config.roomListPageWidth = Math.min(root.defaultPageWidth, root.pageWidth + (mouse.x - _lastX));
                }
            } else if (mouse.x < _lastX) {
                const tmpWidth = root.pageWidth - (_lastX - mouse.x);

                if (tmpWidth < root.minPageWidth) {
                    Config.roomListPageWidth = root.collapsedPageWidth;
                } else {
                    Config.roomListPageWidth = tmpWidth;
                }
            }
        }
    }

    Connections {
        target: root.quitAction
        function onTriggered() {
            Qt.quit()
        }
    }

    property bool startedWithAccountEditor: false

    Component.onCompleted: {
        WindowEffects.setBlur(pageStack, Config.blur && !Config.compactLayout);
        if (AccountService.rowCount() === 0) {
            pageStack.push("qrc:/pages/AddAccounts.qml")
            startedWithAccountEditor = true;
        } else {
            pageStack.push("qrc:/pages/chat/rooms.qml")
        }
    }
    Connections {
        target: Config
        function onBlurChanged() {
            WindowEffects.setBlur(pageStack, Config.blur && !Config.compactLayout);
        }
        function onCompactLayoutChanged() {
            WindowEffects.setBlur(pageStack, Config.blur && !Config.compactLayout);
        }
    }


    Connections {
        target: AccountService
        enabled: startedWithAccountEditor
        onRowsInserted: {
            pageStack.replace("qrc:/pages/chat/rooms.qml")
            startedWithAccountEditor = false;
        }
    }

    //color: Config.blur && !Config.compactLayout ? "transparent" : Kirigami.Theme.backgroundColor

    property Item hoverLinkIndicator: QQC2.Control {
        parent: overlay.parent 
        property alias text: linkText.text
        opacity: text.length > 0 ? 1 : 0

        z: 20
        x: 0
        y: parent.height - implicitHeight
        contentItem: QQC2.Label {
            id: linkText
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
             color: Kirigami.Theme.backgroundColor
        }
    }

    function pushReplaceLayer(page, args) {
        if (pageStack.layers.depth === 2) {
            pageStack.layers.replace(page, args);
        } else {
            pageStack.layers.push(page, args);
        }
    }
    // blur effect
    color: Config.blur && !Config.compactLayout ? "transparent" : Kirigami.Theme.backgroundColor

    // we need to apply the translucency effect separately on top of the color
    background: Rectangle {
        color: Config.blur && !Config.compactLayout ? Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 1 - Config.transparency) : "transparent"
    }
}
