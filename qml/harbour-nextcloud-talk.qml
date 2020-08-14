import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.nextcloud.talk 1.0
import "pages"
import "components"

ApplicationWindow
{
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
    Component.onCompleted: {
        AccountService.loadAccounts();
        if(AccountService.rowCount() === 1) {
            console.log("let's login to an account")
            pageStack.push(Qt.resolvedUrl("./pages/Accounts.qml"), {})
        } else {
            pageStack.push(Qt.resolvedUrl("./pages/chat/rooms.qml"), {})
        }

        //}
    }

    DBusHandler {}
}
