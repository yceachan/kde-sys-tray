// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtQuick.Window
import "../../src/qml" as Tray

Window {
    id: root
    x: 50
    y: 200
    width: 450
    height: 150
    visible: true
    title: "Tray native drag test"

    property bool vertical: false
    property bool reverseLayout: false
    property int itemSize: 32
    property alias applicationDrag: session
    property alias hiddenLayout: hiddenGrid
    property alias visibleLayout: visibleGrid
    property alias applicationHiddenLayout: overflow.layout

    Item { id: visibleGrid; property int currentIndex: -1 }
    Item { id: hiddenGrid; property int currentIndex: -1 }

    Tray.ApplicationDrag {
        id: session
        trayId: "native-test"
        backend: QtObject {
            function moveApplication(itemId, hidden, targetId, after) {
                console.warn("COMMIT", itemId, hidden, targetId, after);
                return true;
            }
        }
    }

    Tray.ApplicationDropArea {
        anchors.fill: parent
        z: -1
        session: session
    }

    Tray.AbstractItem {
        id: app
        x: 20; y: 20; width: 60; height: 60
        index: 0; model: ({ itemType: "StatusNotifier" })
        status: 2; effectiveStatus: 2
        itemId: "panel-app"; text: "Application"
        mainText: ""; subText: ""; textFormat: Text.PlainText
        onClicked: mouse => console.warn("CLICK", mouse.button)
        Rectangle { parent: app.iconContainer; anchors.fill: parent; color: "royalblue" }
    }

    Tray.AbstractItem {
        id: system
        x: 120; y: 20; width: 60; height: 60
        index: 1; model: ({ itemType: "Plasmoid" })
        status: 2; effectiveStatus: 2
        itemId: "system-widget"; text: "System"
        mainText: ""; subText: ""; textFormat: Text.PlainText
        Rectangle { parent: system.iconContainer; anchors.fill: parent; color: "gray" }
    }

    Tray.AbstractItem {
        id: targetApp
        x: 220; y: 20; width: 60; height: 60
        index: 2; model: ({ itemType: "StatusNotifier" })
        status: 2; effectiveStatus: 2
        itemId: "target-app"; text: "Target"
        mainText: ""; subText: ""; textFormat: Text.PlainText
        Rectangle { parent: targetApp.iconContainer; anchors.fill: parent; color: "purple" }
    }

    ListModel {
        id: foldedApplications
        dynamicRoles: true
        Component.onCompleted: append({
            itemType: "StatusNotifier", status: 2, effectiveStatus: 1,
            Id: "folded-app", Title: "Folded", ToolTipTitle: "Folded",
            ToolTipSubTitle: "", IconName: "application-x-executable",
            Icon: "", AttentionIcon: "", AttentionIconName: "", ItemIsMenu: false,
            DataEngineSource: "test"
        })
    }

    Tray.ApplicationOverflow {
        id: overflow
        x: 360; y: 20; width: 60; height: 60
        session: session
        hiddenModel: foldedApplications
        iconSize: root.itemSize
        location: 4 // BottomEdge: popup opens above the panel
        onExpandedChanged: {
            console.warn(expanded ? "OVERFLOW_OPEN" : "OVERFLOW_CLOSED");
            if (expanded) {
                boundsTimer.restart();
            }
        }
    }
    Timer {
        id: boundsTimer
        interval: 100
        onTriggered: {
            const icon = overflow.layout.mapToGlobal(overflow.layout.cellWidth / 2, overflow.layout.cellHeight / 2);
            const blank = overflow.layout.mapToGlobal(overflow.layout.cellWidth * 1.5, overflow.layout.cellHeight / 2);
            console.warn("OVERFLOW_BOUNDS", JSON.stringify({
                appX: Math.round(icon.x), appY: Math.round(icon.y),
                blankX: Math.round(blank.x), blankY: Math.round(blank.y),
                gridWidth: overflow.layout.width, gridHeight: overflow.layout.height
            }));
        }
    }

    Component.onCompleted: console.warn("READY")
}
