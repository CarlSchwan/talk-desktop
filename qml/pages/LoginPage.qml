import QtQuick 2.0
import QtWebKit 3.0
import Sailfish.Silica 1.0
import "../lib/NcAuth.js" as Nc
import sfos.nextcloud.login.flow.demo 1.0

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
                    //webView.url = Nc.getLoginFlowUrl(instance.text);
                    //webView.header.
                    //Nc.triggerLoginFlow(instance.text, webView);
                    var flowUrl = Nc.getLoginFlowUrl(instance.text);
                    console.log("going after " + flowUrl);
                    var r = auth.startLoginFlow(flowUrl);
                    //webView.url = flowUrl;
                    //console.log("got request " + r);
                    //webView.load(r);
                    //console.log("login flow kicked");
                    //webView.visible = true;
                    // TODO: so far we get Access Forbidden, Invalid Request!
                }
            }
        }
    }

    NcAuth {
        id: auth

        onContentChanged: {
            webView.loadHtml(auth.content, auth.url());
        }
    }

    //NcAuthNAM {
    //    id: nam;
    //}

    SilicaWebView {
        id: webView;
        //signal viewReady(var view);

        visible: auth.webVisible;

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Component.onCompleted: {
            //nam.replacePagesQNAM(webView);
        }

        onLoadingChanged: {
            console.log(url);
        }

        PullDownMenu {
            MenuItem {
                text: "Reload"
                onClicked: webView.reload()
            }
        }
    }


}
