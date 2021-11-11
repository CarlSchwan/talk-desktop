# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <fella@posteo.de>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(talk
    db.cpp
    discovery.cpp
    discoveryrun.cpp
    kirigami-nextcloud-talk.cpp
    nextcloudaccount.cpp
    room.cpp
    roomservice.cpp
    roomlistfilter.cpp
    messageeventmodel.cpp
    desktop/chatboxhelper.cpp
    desktop/clipboard.cpp
    desktop/emojimodel.cpp
    desktop/windoweffects.cpp
    services/accounts.cpp
    services/capabilities.cpp
    services/download.cpp
    #services/knotifications.cpp
    services/participants.cpp
    services/requestfactory.cpp
    services/secrets.cpp
    providers/abstractnextcloudimageprovider.cpp
    providers/avatarprovider.cpp
    providers/previewprovider.cpp
    constants/ConversationTypeClass.cpp
    constants/PresenceStatusClass.cpp
    constants/systemmessagetype.cpp
    ../kirigami-qml/res.qrc
)

target_compile_definitions(talk PRIVATE KDE_EDITION)

target_sources(talk PRIVATE ${NEOCHAT_ICON})

if(NOT ANDROID)
    target_sources(talk PRIVATE desktop/colorschemer.cpp)
    target_link_libraries(talk PRIVATE KF5::ConfigWidgets KF5::WindowSystem KF5::SonnetCore)
    target_compile_definitions(talk PRIVATE -DHAVE_COLORSCHEME)
    target_compile_definitions(talk PRIVATE -DHAVE_WINDOWSYSTEM)
endif()

target_include_directories(talk PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(talk PRIVATE Qt::Quick Qt::Concurrent Qt::Qml Qt::Gui Qt::Sql Qt::Network Qt::QuickControls2 KF5::I18n KF5::Kirigami2 KF5::Notifications KF5::ConfigCore KF5::ConfigGui KF5::CoreAddons ${QTKEYCHAIN_LIBRARIES})
kconfig_add_kcfg_files(talk GENERATE_MOC talkconfig.kcfgc)

if(ANDROID)
    target_link_libraries(talk PRIVATE Qt5::Svg OpenSSL::SSL)
    kirigami_package_breeze_icons(ICONS
        "help-about"
        "im-user"
        "im-invisible-user"
        "im-kick-user"
        "mail-attachment"
        "dialog-cancel"
        "preferences-desktop-emoticons"
        "document-open"
        "document-save"
        "document-send"
        "dialog-close"
        "edit-delete-remove"
        "code-context"
        "document-edit"
        "list-user-add"
        "list-add-user"
        "user-others"
        "media-playback-pause"
        "media-playback-start"
        "go-up"
        "go-down"
        "list-add"
        "irc-join-channel"
        "settings-configure"
        "configure"
        "rating"
        "rating-unrated"
        "search"
        "mail-replied-symbolic"
        "edit-copy"
        "gtk-quit"
        "compass"
        "network-connect"
    )
else()
    target_link_libraries(talk PRIVATE Qt5::Widgets KF5::KIOWidgets)
endif()

if(TARGET KF5::DBusAddons)
    target_link_libraries(talk PRIVATE KF5::DBusAddons)
    target_compile_definitions(talk PRIVATE -DHAVE_KDBUSADDONS)
endif()

if (TARGET KF5::KIOWidgets)
    target_compile_definitions(talk PRIVATE -DHAVE_KIO)
endif()

install(TARGETS talk ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
