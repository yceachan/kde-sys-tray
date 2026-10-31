# SPDX-FileCopyrightText: 2026 yceachan
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(XVFB_RUN xvfb-run REQUIRED)
find_program(QML_RUNNER qml HINTS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}" /usr/lib64/qt6/bin REQUIRED)
add_test(NAME native-application-drag
    COMMAND ${XVFB_RUN} -a -s "-screen 0 1280x1024x24"
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/native/drag.py ${QML_RUNNER})
set_tests_properties(native-application-drag PROPERTIES TIMEOUT 45)

find_program(QML_TEST_RUNNER qmltestrunner HINTS /usr/lib64/qt6/bin REQUIRED)
add_test(NAME application-overflow-ui
    COMMAND ${XVFB_RUN} -a -s "-screen 0 1280x1024x24"
        ${QML_TEST_RUNNER} -input ${CMAKE_CURRENT_SOURCE_DIR}/qml)
set_tests_properties(application-overflow-ui PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=xcb;QT_FORCE_STDERR_LOGGING=1" TIMEOUT 30)

add_executable(widgetexplorertest widgetexplorertest.cpp)
target_link_libraries(widgetexplorertest PRIVATE Qt::Quick Qt::Test)
add_dependencies(widgetexplorertest local.plasma.windowstray)
add_test(NAME widgetexplorertest COMMAND widgetexplorertest)
set_tests_properties(widgetexplorertest PROPERTIES ENVIRONMENT "QT_PLUGIN_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY};QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1")

add_executable(trayuittest trayuittest.cpp)
target_link_libraries(trayuittest PRIVATE Qt::Quick Qt::Widgets Qt::Test Plasma::PlasmaQuick KF6::Package)
add_dependencies(trayuittest local.plasma.windowstray)
add_test(NAME trayuittest COMMAND ${XVFB_RUN} -a -s "-screen 0 1280x1024x24" $<TARGET_FILE:trayuittest>)
set_tests_properties(trayuittest PROPERTIES
    ENVIRONMENT "QT_PLUGIN_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY};QT_QPA_PLATFORM=xcb;QT_FORCE_STDERR_LOGGING=1"
    TIMEOUT 30)
