import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

Image {
    property string account;
    property string user;

    //source: user !== "" ? "image://avatar/" + account + "/" + user + "/" : "image://theme/icon-m-people"
    source: "image://avatar/" + account + "/" + user + "/"
    height: Theme.itemSizeExtraSmall
    width: Theme.itemSizeExtraSmall
    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Item {
           width: avatar.width
           height: avatar.height
           Rectangle {
               anchors.centerIn: parent
               width: avatar.adapt ? avatar.width : Math.min(avatar.width, avatar.height)
               height: avatar.adapt ? avatar.height : width
               radius: Math.min(width, height)
           }
       }
    }
}
