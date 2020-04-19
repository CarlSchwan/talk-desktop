import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Image {
    property string account;
    property string fileId;
    property string filePath;
    property int size: Theme.itemSizeLarge

    source: "image://preview/" + account + "/" + fileId + "/"
    sourceSize: size + "x" + size
    height: size
    width: size

    MouseArea {
        anchors.fill: parent;
        onClicked: {
            DownloadService.getFile(parent.filePath, parent.account)
        }
    }
}
