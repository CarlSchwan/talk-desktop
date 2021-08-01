import QtQuick 2.0
import Nemo.DBus 2.0
import Sailfish.Silica 1.0

DBusAdaptor {
    service: 'org.nextcloud.talk'
    iface: 'org.nextcloud.talk'
    path: '/org/nextcloud/talk'

    xml: '  <interface name="org.nextcloud.talk">\n' +
         '    <method name="openConversation">\n' +
         '      <arg name="token" type="s" direction="in"/>\n"' +
         '      <arg name="roomName" type="s" direction="in"/>\n"' +
         '      <arg name="accountId" type="y" direction="in"/>\n"' +
         '      <arg name="accountUserId" type="s" direction="in"/>\n"' +
         '    </method">\n' +
         '  </interface>\n'

    function openConversation(token, roomName, accountId, accountUserId) {
        pageStack.clear()
        pageStack.push(Qt.resolvedUrl("../pages/chat/rooms.qml"))
        pageStack.push(Qt.resolvedUrl("../pages/chat/room.qml"),
           {
               "token": token,
               "roomName": roomName,
               "accountId": accountId,
               "accountUserId": accountUserId,
           }
        );
        __silica_applicationwindow_instance.activate()
    }
}
