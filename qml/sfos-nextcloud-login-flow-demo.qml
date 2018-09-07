import QtQuick 2.0
import Sailfish.Silica 1.0
import "./lib/Config.js" as Config
import "pages"

ApplicationWindow
{
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
    Component.onCompleted: {
        var obj = {};
        Config.mediator.installTo(obj);
        obj.subscribe('confLoaded', function(){
            console.log('confLoaded');
            if(Config.hasValidInstance()) {
                console.log("at least one working account")
                pageStack.push(Qt.resolvedUrl("./pages/MainPage.qml"), {})
            } else {
                console.log("let's login to an account")
                pageStack.push(Qt.resolvedUrl("./pages/Accounts.qml"), {})
            }

        });
        Config.init()
    }
}
