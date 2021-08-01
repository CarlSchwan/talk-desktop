import QtQuick 2.6
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Page {
    id: accountSettingsPage
    allowedOrientations: Orientation.All

    property variant accountIndex

    function getFeature(role) {
        return AccountService.data(accountIndex, role)
    }

    Connections {
        target: AccountService
        onDataChanged: {
            colorIndicatorColor.color = getFeature(AccountService.ColorRole)
            labelColorMode.accountActiveColorMode = getFeature(AccountService.ColorModeRole)
            buttonResetColor.accountActiveColorMode = getFeature(AccountService.ColorModeRole)
        }
    }

    SilicaFlickable {
        leftMargin: Theme.horizontalPageMargin
        rightMargin: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin

        PageHeader {
            id: header
            title: "Account settings"
        }

        Row {
            id: accountBaseRow
            anchors.top: header.bottom
            width: parent.width

            Image {
                id: accountLogo
                source: getFeature(AccountService.LogoRole)
                width: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
                fillMode: Image.PreserveAspectFit
            }

            Label {
                id: accountName
                text: getFeature(AccountService.NameRole)
                leftPadding: Theme.paddingMedium
                font.pixelSize: Theme.fontSizeLarge
                truncationMode: TruncationMode.Fade
                verticalAlignment: Qt.AlignVCenter
                height: Theme.iconSizeLarge
                width: parent.width - accountLogo.width
                maximumLineCount: 1
            }
        }

        SectionHeader {
            text: "Indicator color"
            id: sectionIndicator
            anchors.top: accountBaseRow.bottom
        }

        Row {
            id: rowColor
            anchors.top: sectionIndicator.bottom
            width: parent.width

            Rectangle {
                id: colorIndicatorColor
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                color: getFeature(AccountService.ColorRole)
                MouseArea {
                    id: colorIndicatorColorTap
                    anchors.fill: parent
                    onClicked: {
                        var colorPicker = pageStack.push("Sailfish.Silica.ColorPickerDialog")
                        colorPicker.accepted.connect(function() {
                            AccountService.setData(accountIndex, colorPicker.color, AccountService.ColorRole)
                        })
                    }
                }
            }

            Label {
                id: labelColorMode
                property int accountActiveColorMode: getFeature(AccountService.ColorModeRole)

                leftPadding: Theme.paddingMedium
                width: parent.width - 2 * Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                verticalAlignment: Qt.AlignVCenter
                text: accountActiveColorMode === AccountService.InstanceColor
                    ? qsTr("from instance theme")
                    : qsTr("selected by you")
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            IconButton {
                id: buttonResetColor
                property int accountActiveColorMode: getFeature(AccountService.ColorModeRole)

                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                icon.source: "image://theme/icon-m-backspace"
                visible: accountActiveColorMode === AccountService.OverriddenColor
                onClicked: AccountService.setData(accountIndex, "", AccountService.ColorRole)
            }
        }


    }
}
