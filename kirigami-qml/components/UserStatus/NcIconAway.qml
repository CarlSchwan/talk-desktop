import QtQuick 2.0
import Sailfish.Silica 1.0

NcIconBackground {
    Rectangle {
        id: statusAwayFullMoon

        height: parent.height - Theme.paddingSmall
        width: parent.width - Theme.paddingSmall

        anchors.centerIn: parent

        color: '#f4a331'
        radius: width
    }

    Rectangle {
        height: statusAwayFullMoon.height * 0.67
        width: statusAwayFullMoon.width * 0.67

        radius: width

        anchors {
            right: statusAwayFullMoon.right
            verticalCenter: statusAwayFullMoon.verticalCenter
        }

        color: parent.backgroundColor
    }

}
