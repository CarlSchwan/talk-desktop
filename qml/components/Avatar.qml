import QtQuick 2.6
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

Image {
    readonly property string objectName: "Avatar"

    // underscored proprties for use within a Loader as Delegate
    property string account: (typeof (_account) !== "undefined") ? _account : undefined
    property string user: (typeof (_user) !== "undefined") ? _user : undefined
    property int size: (typeof (_size) !== "undefined") ? _size : Theme.itemSizeExtraSmall

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
