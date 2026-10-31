# SPDX-FileCopyrightText: 2026 yceachan
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.25)
project(kde-sys-tray VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ECM 6.30 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQmlModule)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)
include(CTest)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets DBus Quick)
find_package(Qt6GuiPrivate REQUIRED NO_MODULE)
find_package(KF6 6.30 REQUIRED COMPONENTS Config CoreAddons IconThemes I18n KIO JobWidgets Package Service WindowSystem XmlGui)
find_package(Plasma 6.7.5 EXACT REQUIRED)
find_package(PlasmaQuick 6.7.5 EXACT REQUIRED)
find_path(KSTATUSNOTIFIERITEM_DBUS_INTERFACES_DIR kf6_org.kde.StatusNotifierItem.xml
    PATHS /usr/share/dbus-1/interfaces REQUIRED)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include_directories("${CMAKE_BINARY_DIR}")
configure_file(config-X11.h.in config-X11.h)
add_subdirectory(libdbusmenuqt)
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
