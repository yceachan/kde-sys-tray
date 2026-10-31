// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
pragma ComponentBehavior: Bound

import QtQuick

// A persistent native drag source: popup and panel are different Wayland surfaces.
// Model delegates may be recreated only after the native drag has finished.
Item {
    id: session

    required property string trayId
    required property var backend
    property string applicationId: ""
    readonly property bool dragging: proxy.Drag.active
    property var pendingDrop: null

    readonly property string itemMime: "application/x-windowstray-item"
    readonly property string trayMime: "application/x-windowstray-instance"

    function start(itemId, icon, mouseArea) {
        if (applicationId !== "") {
            return;
        }
        applicationId = itemId;
        pendingDrop = null;
        const grabbed = icon.grabToImage(result => {
            if (!mouseArea || !mouseArea.pressed || mouseArea.pressedButtons !== Qt.LeftButton) {
                applicationId = "";
                return;
            }
            proxy.Drag.imageSource = result.url;
            proxy.Drag.active = true;
        });
        if (!grabbed) {
            applicationId = "";
            console.warn("Unable to capture the application drag icon");
        }
    }

    function accepts(event) {
        return dragging && applicationId !== ""
            && event.formats.indexOf(itemMime) >= 0
            && event.formats.indexOf(trayMime) >= 0
            && event.getDataAsString(itemMime) === applicationId
            && event.getDataAsString(trayMime) === trayId;
    }

    function queueDrop(event, hidden, targetId, after) {
        if (!accepts(event)) {
            return false;
        }
        pendingDrop = { itemId: applicationId, hidden: hidden, targetId: targetId, after: after };
        return true;
    }

    function finish(action) {
        const change = pendingDrop;
        pendingDrop = null;
        proxy.Drag.active = false;
        applicationId = "";
        if (action === Qt.MoveAction && change) {
            Qt.callLater(() => {
                if (!session.backend.moveApplication(change.itemId, change.hidden, change.targetId, change.after)) {
                    console.warn("Application tray drop rejected: source or target is no longer available");
                }
            });
        }
    }

    Item {
        id: proxy
        width: 32
        height: 32
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.MoveAction
        Drag.proposedAction: Qt.MoveAction
        Drag.mimeData: ({
            [session.itemMime]: session.applicationId,
            [session.trayMime]: session.trayId
        })
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.onDragFinished: action => session.finish(action)
    }
}
