# Nextcloud Talk for Desktop/Mobile Linux

Nextcloud Talk is a fully on-premises audio/video and chat communication service. It features web and mobile apps and is designed to offer the highest degree of security while being easy to use.

Nextcloud Talk lowers the barrier for communication and lets your team connect any time, any where, on any device, with each other, customers or partners.

## Build

Require: kirigami, qqc2-desktop-style and kwindowsystem and qt5.15 and Qt5Keychain (you need the devel package)

```sh
cmake -DCMAKE_INSTALL_PREFIX=~/kde/usr -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_COMPILER=clazy -DCMAKE_BUILD_TYPE=Debug -S . build
cmake --build build
cmake --install build
source build/prefix.sh

```

## Scope

This client for Nextcloud Talk is a hobby project in development and currently in an alpha stage. The goal is to have a native desktop and Plasma Mobile client using Kirigami, a convergent framework developed by the KDE Community.

Due to the earlier development stage, plus refrain from sending bug reports. Code contributions however are very welcome!

## Credit

This app wouldn't have been possible without the amazing work of blizzz with his app for
[SailfishOS](https://codeberg.org/blizzz/harbour-nextcloud-talk)
