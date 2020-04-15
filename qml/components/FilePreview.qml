import QtQuick 2.0
import Sailfish.Silica 1.0

Image {
    property string account;
    property string fileId;
    property int size: Theme.itemSizeLarge

    source: "image://preview/" + account + "/" + fileId + "/"
    sourceSize: size + "x" + size
    height: size
    width: size
}
