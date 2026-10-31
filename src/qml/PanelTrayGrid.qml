/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Konrad Materka <materka@gmail.com>
    SPDX-FileCopyrightText: 2026 Nathaniel Krebs <areyoufeelingitnowmrkrebs@gmail.com>
    SPDX-FileCopyrightText: 2026 yceachan

    SPDX-License-Identifier: LGPL-2.0-or-later
    Modified 2026-10-31: extract upstream panel geometry for separate application and system grids.
*/
// Panel geometry follows the upstream system tray grid.
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid

GridView {
    id: grid
    required property bool vertical
    required property bool reverseLayout
    required property int panelThickness
    required property bool autoSize
    required property int iconSpacing

    interactive: false
    flow: vertical ? GridView.LeftToRight : GridView.TopToBottom
    verticalLayoutDirection: vertical && reverseLayout ? GridView.BottomToTop : GridView.TopToBottom

    readonly property int smallIconSize: Kirigami.Units.iconSizes.smallMedium
    readonly property int rowsOrColumns: autoSize ? 1
        : Math.max(1, Math.min(count, Math.floor(panelThickness / (smallIconSize + Kirigami.Units.smallSpacing))))
    readonly property int cellSpacing: Kirigami.Units.smallSpacing * iconSpacing
    readonly property int smallSizeCellLength: panelThickness < smallIconSize ? smallIconSize : smallIconSize + cellSpacing
    readonly property int itemSize: autoSize
        ? Kirigami.Units.iconSizes.roundedIconSize(Math.min(panelThickness, Kirigami.Units.iconSizes.enormous))
        : smallIconSize

    cellHeight: vertical ? (autoSize ? itemSize + (panelThickness < itemSize ? 0 : cellSpacing) : smallSizeCellLength)
        : Math.floor(panelThickness / rowsOrColumns)
    cellWidth: vertical ? Math.floor(panelThickness / rowsOrColumns)
        : (autoSize ? itemSize + (panelThickness < itemSize ? 0 : cellSpacing) : smallSizeCellLength)

    implicitHeight: vertical ? cellHeight * Math.ceil(count / rowsOrColumns) : panelThickness
    implicitWidth: vertical ? panelThickness : cellWidth * Math.ceil(count / rowsOrColumns)

    delegate: ItemLoader {
        id: delegate
        width: grid.cellWidth
        height: grid.cellHeight

        Component.onCompleted: {
            let sibling = grid.itemAtIndex(index - 1);
            if (sibling) {
                Plasmoid.stackItemAfter(delegate, sibling);
            } else {
                sibling = grid.itemAtIndex(index + 1);
                if (sibling) {
                    Plasmoid.stackItemBefore(delegate, sibling);
                }
            }
        }
    }
}
