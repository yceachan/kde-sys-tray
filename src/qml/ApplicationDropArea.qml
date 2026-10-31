// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

DropArea {
    id: area
    required property ApplicationDrag session
    property bool hiddenPlacement: false
    property string targetItemId: ""
    property bool verticalInsertion: false
    property bool reverseInsertion: false
    property bool insertAfter: false

    keys: [session.itemMime]

    function updateInsertion(event) {
        const secondHalf = verticalInsertion ? event.y >= height / 2 : event.x >= width / 2;
        insertAfter = secondHalf !== reverseInsertion;
    }

    onEntered: event => {
        event.accepted = session.accepts(event);
        updateInsertion(event);
    }
    onPositionChanged: event => updateInsertion(event)
    onDropped: event => {
        updateInsertion(event);
        if (session.queueDrop(event, hiddenPlacement, targetItemId, insertAfter)) {
            event.accept(Qt.MoveAction);
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: area.containsDrag
        color: Kirigami.Theme.highlightColor
        opacity: 0.22
        radius: Kirigami.Units.smallSpacing
    }
    Rectangle {
        visible: area.containsDrag && area.targetItemId !== ""
        color: Kirigami.Theme.highlightColor
        width: area.verticalInsertion ? parent.width : 2
        height: area.verticalInsertion ? 2 : parent.height
        x: !area.verticalInsertion && (area.insertAfter !== area.reverseInsertion) ? parent.width - width : 0
        y: area.verticalInsertion && (area.insertAfter !== area.reverseInsertion) ? parent.height - height : 0
    }
}
