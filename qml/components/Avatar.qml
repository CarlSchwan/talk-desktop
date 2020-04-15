import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

Image {
    property string account;
    property string user;
    property int size: Theme.itemSizeExtraSmall

    source: "image://avatar/" + account + "/" + user + "/"
    height: size
    width: size
    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Item {
           width: size
           height: size
           Rectangle {
               anchors.centerIn: parent
               width: size
               height: size
               radius: Math.min(width, height)
           }
       }
    }
}
