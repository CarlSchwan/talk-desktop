// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Qt.labs.platform 1.1

Image {
    id: img

    readonly property bool isAnimated: model.contentType === "image/gif"

    property bool openOnFinished: false
    property bool readonly: false

    source: model.filePreviewUrl

    fillMode: Image.PreserveAspectFit

    HoverHandler {
        id: hoverHandler
        enabled: img.readonly
    }

    function saveFileAs() {
        const dialog = fileDialog.createObject(ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
    }

    Component {
        id: fileDialog

        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
            onAccepted: {
                currentRoom.downloadFile(eventId, file)
            }
        }
    }

    function downloadAndOpen() {
        if (downloaded) {
            openSavedFile();
        } else {
            openOnFinished = true
            currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile() {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }
}
