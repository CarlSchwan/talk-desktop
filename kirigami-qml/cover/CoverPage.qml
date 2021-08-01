import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {

    Image {
        anchors {
            bottom : parent.bottom
            left: parent.left
            right: parent.right
            top: parent.top
        }
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter
        fillMode: Image.PreserveAspectFit
        source: "../images/cover.svg"
    }
}
