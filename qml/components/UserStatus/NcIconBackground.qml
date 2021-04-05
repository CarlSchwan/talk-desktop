import QtQuick 2.0
import Sailfish.Silica 1.0

SilicaItem {
    height: Theme.itemSizeExtraSmall
    width: Theme.itemSizeExtraSmall

    property string backgroundColor: Theme.colorScheme === Theme.DarkOnLight ? '#ffffff' : '#000000'

    Rectangle {
        id: statusIconBackground

        anchors.fill: parent
        radius: width

        color: backgroundColor
    }
}
