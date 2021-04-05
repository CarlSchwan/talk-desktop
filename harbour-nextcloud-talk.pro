# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-nextcloud-talk

CONFIG += sailfishapp
PKGCONFIG += nemonotifications-qt5 sailfishsecrets
QT += sql dbus

notificationcategories.files=$$PWD/notificationcategories/*.conf
notificationcategories.path=/usr/share/lipstick/notificationcategories

events.files=$$PWD/events/*.ini
events.path=/usr/share/ngfd/events.d

INSTALLS += notificationcategories events

SOURCES += src/harbour-nextcloud-talk.cpp \
    src/db.cpp \
    src/nextcloudaccount.cpp \
    src/discovery.cpp \
    src/discoveryrun.cpp \
    src/providers/abstractnextcloudimageprovider.cpp \
    src/providers/avatarprovider.cpp \
    src/providers/previewprovider.cpp \
    src/roomservice.cpp \
    src/room.cpp \
    src/services/accounts.cpp \
    src/services/capabilities.cpp \
    src/services/download.cpp \
    src/services/notifications.cpp \
    src/services/participants.cpp \
    src/services/requestfactory.cpp \
    src/services/secrets.cpp

DISTFILES += qml/lib/API.js \
    events/nextcloud_talk_im.ini \
    notificationcategories/x-nextcloud.talk.im.conf \
    qml/components/Avatar.qml \
    qml/components/ConversationIcon.qml \
    qml/components/DBusHandler.qml \
    qml/components/FilePreview.qml \
    qml/components/UserStatus/NcIconAway.qml \
    qml/components/UserStatus/NcIconBackground.qml \
    qml/components/UserStatus/NcIconDnd.qml \
    qml/components/UserStatus/NcIconOnline.qml \
    qml/harbour-nextcloud-talk.qml \
    qml/cover/CoverPage.qml \
    qml/pages/accounts/AccountSettings.qml \
    qml/pages/chat/participants.qml \
    rpm/harbour-nextcloud-talk.changes.in \
    rpm/harbour-nextcloud-talk.changes.run.in \
    rpm/harbour-nextcloud-talk.spec \
    rpm/harbour-nextcloud-talk.yaml \
    translations/*.ts \
    harbour-nextcloud-talk.desktop \
    qml/pages/Accounts.qml \
    qml/pages/LegacyAddAccount.qml \
    qml/pages/chat/rooms.qml \
    qml/pages/chat/room.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-nextcloud-talk-de.ts

HEADERS += \
    src/constants/ConversationTypeClass.h \
    src/constants/PresenceStatusClass.h \
    src/db.h \
    src/nextcloudaccount.h \
    src/discovery.h \
    src/discoveryrun.h \
    src/providers/abstractnextcloudimageprovider.h \
    src/providers/avatarprovider.h \
    src/providers/previewprovider.h \
    src/roomservice.h \
    src/room.h \
    src/services/accounts.h \
    src/services/capabilities.h \
    src/services/download.h \
    src/services/notifications.h \
    src/services/participants.h \
    src/services/requestfactory.h \
    src/services/secrets.h
