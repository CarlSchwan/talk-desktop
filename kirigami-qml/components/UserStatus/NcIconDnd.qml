import QtQuick 2.0
import Sailfish.Silica 1.0

NcIconBackground {
    Rectangle {
        id: statusDndDisc

        height: parent.height - Theme.paddingSmall
        width: parent.width - Theme.paddingSmall

        anchors.centerIn: parent

        color: '#ed484c'
        radius: width
    }

    Rectangle {
        height: statusDndDisc.height * 0.33
        width: statusDndDisc.width * 0.67
        anchors.centerIn: statusDndDisc

        radius: height * 0.3

        color: '#fffafa'
    }

}
