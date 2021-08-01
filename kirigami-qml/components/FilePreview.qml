import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0

Row {
    property string account;
    property string fileId;
    property string filePath;
    property int size: Theme.itemSizeLarge

    function updateAction() {
        fileAction.icon.source = DownloadService.fileExists(filePath, account)
            ? "image://theme/icon-m-file-other-light"
            : "image://theme/icon-m-cloud-download"
    }

    function clickHandler() {
        if(DownloadService.fileExists(filePath, account)) {
            Qt.openUrlExternally(DownloadService.filePath(filePath, account))
        } else {
            DownloadService.getFile(filePath, account)
        }
    }

    Image {
        source: "image://preview/" + account + "/" + fileId + "/"
        sourceSize: size + "x" + size
        height: size
        width: size

        MouseArea {
            anchors.fill: parent;
            onClicked: clickHandler()
        }
    }

    IconButton {
        id: fileAction
        icon.source: DownloadService.fileExists(parent.filePath, parent.account)
                        ? "image://theme/icon-m-file-other-light"
                        : "image://theme/icon-m-cloud-download"
        onClicked: clickHandler()
        height: size
        width: size
    }

    Connections {
        target: DownloadService
        onFileDownloaded: updateAction()
    }
}
