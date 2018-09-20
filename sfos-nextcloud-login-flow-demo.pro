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
TARGET = sfos-nextcloud-login-flow-demo

CONFIG += sailfishapp

SOURCES += src/sfos-nextcloud-login-flow-demo.cpp \
    src/ncauth.cpp \
    src/ncauthnetworkaccessmanager.cpp \
    src/ncauthnamf.cpp \
    src/accountmodel.cpp \
    src/nextcloudaccount.cpp \
    src/discovery.cpp \
    src/discoveryrun.cpp \
    src/accountreader.cpp

DISTFILES += qml/lib/API.js \
    qml/sfos-nextcloud-login-flow-demo.qml \
    qml/cover/CoverPage.qml \
    rpm/sfos-nextcloud-login-flow-demo.changes.in \
    rpm/sfos-nextcloud-login-flow-demo.changes.run.in \
    rpm/sfos-nextcloud-login-flow-demo.spec \
    rpm/sfos-nextcloud-login-flow-demo.yaml \
    translations/*.ts \
    sfos-nextcloud-login-flow-demo.desktop \
    qml/pages/LoginPage.qml \
    qml/lib/NcAuth.js \
    qml/pages/Accounts.qml \
    qml/pages/LegacyAddAccount.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/sfos-nextcloud-login-flow-demo-de.ts

HEADERS += \
    src/ncauth.h \
    src/ncauthnetworkaccessmanager.h \
    src/ncauthnamf.h \
    src/accountmodel.h \
    src/nextcloudaccount.h \
    src/discovery.h \
    src/discoveryrun.h \
    src/accountreader.h
