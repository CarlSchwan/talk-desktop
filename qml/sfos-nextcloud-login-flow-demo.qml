import QtQuick 2.0
import Sailfish.Silica 1.0
import sfos.nextcloud.login.flow.demo 1.0
import "pages"

ApplicationWindow
{
    AccountModel {
        id: accounts
    }

    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
    Component.onCompleted: {
        if(accounts.rowCount() === 0) {
            console.log("let's login to an account")
            pageStack.push(Qt.resolvedUrl("./pages/Accounts.qml"), {})
        } else {
            pageStack.push(Qt.resolvedUrl("./pages/chat/rooms.qml"), {})
        }

        //}
    }
}
