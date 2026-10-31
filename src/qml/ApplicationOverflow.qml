// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore

Item {
    id: overflow
    required property ApplicationDrag session
    required property var hiddenModel
    required property int location
    property int iconSize: Kirigami.Units.iconSizes.smallMedium
    property bool expanded: false
    property bool openedByHover: false
    readonly property alias layout: hiddenGrid
    readonly property alias popup: popup
    readonly property alias button: button

    implicitWidth: iconSize + Kirigami.Units.smallSpacing * 2
    implicitHeight: implicitWidth

    function scheduleClose() {
        if (openedByHover) {
            closeTimer.restart();
        }
    }

    function openOnHover() {
        if (!expanded) {
            openedByHover = true;
            expanded = true;
        }
    }

    onExpandedChanged: if (!expanded) openedByHover = false

    Connections {
        target: overflow.session
        function onDraggingChanged() {
            if (!overflow.session.dragging) {
                overflow.scheduleClose();
            }
        }
    }

    Timer {
        id: openTimer
        interval: 300
        onTriggered: overflow.openOnHover()
    }
    Timer {
        id: closeTimer
        interval: 500
        onTriggered: {
            if (!arrowHover.hovered && !popupHover.hovered && !overflow.session.dragging) {
                overflow.expanded = false;
            }
        }
    }

    HoverHandler {
        id: arrowHover
        onHoveredChanged: {
            if (hovered) {
                openTimer.restart();
            } else {
                openTimer.stop();
                overflow.scheduleClose();
            }
        }
    }

    PlasmaComponents.ToolButton {
        id: button
        objectName: "applicationOverflowButton"
        anchors.centerIn: parent
        width: overflow.implicitWidth
        height: Math.max(width, overflow.height)
        icon.width: overflow.iconSize
        icon.height: overflow.iconSize
        icon.name: {
            const opening = !overflow.expanded;
            switch (overflow.location) {
            case PlasmaCore.Types.TopEdge: return opening ? "arrow-down-symbolic" : "arrow-up-symbolic";
            case PlasmaCore.Types.LeftEdge: return opening ? "arrow-right-symbolic" : "arrow-left-symbolic";
            case PlasmaCore.Types.RightEdge: return opening ? "arrow-left-symbolic" : "arrow-right-symbolic";
            default: return opening ? "arrow-up-symbolic" : "arrow-down-symbolic";
            }
        }
        display: PlasmaComponents.AbstractButton.IconOnly
        text: qsTr("Hidden applications")
        Accessible.name: text
        checked: overflow.expanded
        onClicked: {
            openTimer.stop();
            overflow.openedByHover = false;
            overflow.expanded = !overflow.expanded;
        }

        // A frame makes the application entrance distinct from the system arrow.
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: Kirigami.Units.smallSpacing
            color: "transparent"
            border.color: Kirigami.Theme.textColor
            opacity: 0.22
        }

        PlasmaComponents.ToolTip {
            text: button.text
        }
        ApplicationDropArea {
            anchors.fill: parent
            session: overflow.session
            hiddenPlacement: true
            onContainsDragChanged: {
                if (containsDrag) {
                    openTimer.restart();
                } else {
                    openTimer.stop();
                    overflow.scheduleClose();
                }
            }
        }
    }

    PlasmaCore.AppletPopup {
        id: popup
        objectName: "applicationOverflowPopup"
        // Deliberately has no appletInterface: this compact popup must not read
        // or overwrite the official system popup's persisted width and height.
        visualParent: button
        popupDirection: {
            switch (overflow.location) {
            case PlasmaCore.Types.TopEdge: return Qt.BottomEdge;
            case PlasmaCore.Types.LeftEdge: return Qt.RightEdge;
            case PlasmaCore.Types.RightEdge: return Qt.LeftEdge;
            default: return Qt.TopEdge;
            }
        }
        hideOnWindowDeactivate: !overflow.session.dragging
        visible: overflow.expanded
        onVisibleChanged: {
            if (!visible) {
                overflow.expanded = false;
            } else if (!overflow.session.dragging) {
                requestActivate();
                hiddenGrid.forceActiveFocus();
            }
        }

        mainItem: PlasmaComponents.ScrollView {
            id: content
            readonly property int columns: 5
            readonly property int cellSize: overflow.iconSize + Kirigami.Units.smallSpacing * 4
            implicitWidth: columns * cellSize + Kirigami.Units.smallSpacing * 2
            implicitHeight: Math.max(1, Math.min(5, Math.ceil(hiddenGrid.count / columns))) * cellSize
                + Kirigami.Units.smallSpacing * 2
            padding: Kirigami.Units.smallSpacing
            background: null

            HoverHandler {
                id: popupHover
                onHoveredChanged: {
                    if (hovered) {
                        closeTimer.stop();
                    } else {
                        overflow.scheduleClose();
                    }
                }
            }
            Keys.onEscapePressed: overflow.expanded = false

            GridView {
                id: hiddenGrid
                objectName: "applicationOverflowGrid"
                width: content.availableWidth
                height: content.availableHeight
                model: overflow.hiddenModel
                cellWidth: content.cellSize
                cellHeight: content.cellSize
                clip: true
                currentIndex: -1
                keyNavigationEnabled: true
                activeFocusOnTab: true

                ApplicationDropArea {
                    objectName: "applicationOverflowDropArea"
                    parent: hiddenGrid
                    anchors.fill: parent
                    z: -1
                    session: overflow.session
                    hiddenPlacement: true
                }
                delegate: ItemLoader {
                    width: hiddenGrid.cellWidth
                    height: hiddenGrid.cellHeight
                    compactHidden: true
                }

                PlasmaComponents.Label {
                    parent: hiddenGrid
                    anchors.centerIn: parent
                    visible: hiddenGrid.count === 0
                    text: qsTr("Drag applications here to hide them")
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    width: hiddenGrid.width - Kirigami.Units.largeSpacing
                    enabled: false
                }
            }
        }
    }
}
