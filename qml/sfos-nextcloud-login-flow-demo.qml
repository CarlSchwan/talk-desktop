import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow
{
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
    Component.onCompleted: {
        //if(Config.hasValidInstance()) { ← pseudo
        //    console.log("at least one working account")
        //    pageStack.push(Qt.resolvedUrl("./pages/MainPage.qml"), {})
        //} else {
            console.log("let's login to an account")
            pageStack.push(Qt.resolvedUrl("./pages/Accounts.qml"), {})
        //}
    }
}
