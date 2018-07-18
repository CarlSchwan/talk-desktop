import QtQuick 2.0
import QtWebKit 3.0
import Sailfish.Silica 1.0
import "../lib/NcAuth.js" as Nc

Page {
    id: loginPage

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Login") }

            SectionHeader {
                text: "Instance"
            }

            TextField {
                id: instance
                focus: true
                label: qsTr("Enter a Nextcloud host")
                placeholderText: label
                text: "https://o.schiwon.me"
                width: parent.width
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    console.log("onclick")
                    //webView.url = Nc.getLoginFlowUrl(instance.text);
                    //webView.header.
                    Nc.triggerLoginFlow(instance.text, webView);
                    webView.visible = true;
                    // TODO: so far we get Access Forbidden, Invalid Request!
                }
            }
        }
    }

    SilicaWebView {
        id: webView
        visible: false
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        onLoadingChanged: {
            console.log(url);
            console.log(data);
        }

        PullDownMenu {
            MenuItem {
                text: "Reload"
                onClicked: webView.reload()
            }
        }
    }
}
