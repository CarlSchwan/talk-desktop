import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0

Page {
    id: room
    allowedOrientations: Orientation.All

    property Chat room;

    onStatusChanged: {
        if(status === PageStatus.Activating) {
        }
    }

    SilicaListView {
        id: chat
        anchors.fill: parent

        header: PageHeader {
            id: header
        }

        delegate: BackgroundItem {
            Label {
                text: name
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingLarge
                }
            }
        }

        model: room

        VerticalScrollDecorator {}
    }

}
