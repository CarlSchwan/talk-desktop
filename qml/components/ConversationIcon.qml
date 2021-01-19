import QtQuick 2.0
import harbour.nextcloud.talk 1.0
import Sailfish.Silica 1.0

Image {
    readonly property string objectName: "ConversationIcon"

    property int conversationType: _conversationType ? _conversationType : undefined

    source: {
        switch (conversationType) {
            case ConversationType.Public:
                return "image://theme/icon-m-region"

            case ConversationType.Group:
                return "image://theme/icon-m-users"

            case ConversationType.Changelog:
                return Qt.resolvedUrl("../images/cover.svg")

            case ConversationType.Unknown:
            default:
                return "image://theme/icon-m-chat"
        }
    }

    width: Theme.iconSizeMedium
    fillMode: Image.PreserveAspectFit
    height: roomName.paintedHeight + lastMessage.paintedHeight
    verticalAlignment: Qt.AlignVCenter
}
