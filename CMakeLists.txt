# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <fella@posteo.de>
# SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.1)

project(NextcloudTalk)

set(KF5_MIN_VERSION "5.82.0")
set(QT_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMQMLModules)
include(KDEClangFormat)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)

#if(NEOCHAT_FLATPAK)
#    include(cmake/Flatpak.cmake)
#endif()

# Fix a crash due to problems with quotient's event system. Can probably be removed once the reworked event system is in
cmake_policy(SET CMP0063 OLD)

ecm_setup_version(1.2.80
    VARIABLE_PREFIX TALK
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/talk-version.h
)

find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Core Quick Gui QuickControls2 Multimedia Svg Sql)
set_package_properties(Qt5 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
find_package(KF5 ${KF5_MIN_VERSION} COMPONENTS Kirigami2 I18n Notifications Config CoreAddons)
set_package_properties(KF5 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
set_package_properties(KF5Kirigami2 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)

find_package(Qt5Keychain)
set_package_properties(Qt5Keychain PROPERTIES
    TYPE REQUIRED
    PURPOSE "Secure storage of account secrets"
)

if(ANDROID)
    find_package(OpenSSL)
    set_package_properties(OpenSSL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Encrypted communications"
    )
else()
    find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Widgets)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS QQC2DesktopStyle ConfigWidgets KIO WindowSystem Config)
    set_package_properties(KF5QQC2DesktopStyle PROPERTIES
        TYPE RUNTIME
    )
endif()

ecm_find_qmlmodule(org.kde.kitemmodels 1.0)


install(FILES org.nextcloud.talk.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.nextcloud.talk.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# add_definitions(-DQT_NO_FOREACH) TODO cleanup old foreach

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

#file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
#kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
