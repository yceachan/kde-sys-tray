// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtTest
import org.kde.plasma.core as PlasmaCore
import "../../src/qml" as Tray

Item {
    id: root
    width: 500
    height: 180

    property bool vertical: false
    property bool reverseLayout: false
    property int itemSize: 22
    property alias applicationDrag: session
    property alias hiddenLayout: systemHiddenGrid
    property alias applicationHiddenLayout: overflow.layout

    Item { id: systemHiddenGrid; property int currentIndex: -1 }
    QtObject {
        id: backend
        function moveApplication(id, hidden, target, after) { return true; }
    }
    Tray.ApplicationDrag { id: session; trayId: "test"; backend: backend }

    ListModel { id: source }
    ListModel { id: emptyApplications }
    Tray.TrayModels { id: models; source: source }

    Tray.ApplicationOverflow {
        id: overflow
        x: 250; y: 120
        width: 32; height: 32
        session: session
        hiddenModel: emptyApplications
        iconSize: root.itemSize
        location: PlasmaCore.Types.BottomEdge
    }

    Tray.AbstractItem {
        id: sizingProbe
        x: 20; y: 20
        width: overflow.layout.cellWidth
        height: overflow.layout.cellHeight
        index: 0
        model: ({itemType: "StatusNotifier"})
        itemId: "size-probe"
        status: PlasmaCore.Types.ActiveStatus
        effectiveStatus: PlasmaCore.Types.PassiveStatus
        compactHidden: true
        text: "Size probe"
        mainText: ""
        subText: ""
        textFormat: Text.PlainText
    }

    TestCase {
        name: "ApplicationOverflow"
        when: windowShown

        function init() {
            root.itemSize = 22;
            source.clear();
            overflow.expanded = false;
            mouseMove(root, 0, 0);
        }

        function test_separateModels() {
            source.append({itemType: "StatusNotifier", itemId: "visible-app", effectiveStatus: PlasmaCore.Types.ActiveStatus});
            source.append({itemType: "StatusNotifier", itemId: "hidden-app", effectiveStatus: PlasmaCore.Types.PassiveStatus});
            source.append({itemType: "StatusNotifier", itemId: "disabled-app", effectiveStatus: PlasmaCore.Types.HiddenStatus});
            source.append({itemType: "Plasmoid", itemId: "network", effectiveStatus: PlasmaCore.Types.ActiveStatus});
            source.append({itemType: "Plasmoid", itemId: "vault", effectiveStatus: PlasmaCore.Types.PassiveStatus});
            source.append({itemType: "BackgroundApp", itemId: "background", effectiveStatus: PlasmaCore.Types.PassiveStatus});
            compare(models.visibleApplications.count, 1);
            compare(models.hiddenApplications.count, 1);
            compare(models.visibleSystems.count, 1);
            compare(models.hiddenSystems.count, 2);

            source.setProperty(0, "effectiveStatus", PlasmaCore.Types.PassiveStatus);
            compare(models.visibleApplications.count, 0);
            compare(models.hiddenApplications.count, 2);
            compare(models.hiddenSystems.count, 2); // application folding never enters the system popup
        }

        function test_emptyEntranceAndHover() {
            verify(overflow.button.visible);
            verify(overflow.button.width > 0);
            verify(overflow.button.height > 0);
            compare(overflow.layout.count, 0);
            mouseMove(overflow.button, overflow.button.width / 2, overflow.button.height / 2);
            tryCompare(overflow, "expanded", true, 1500);
            verify(overflow.popup.visible);
            verify(overflow.layout.width > 0);
            verify(overflow.layout.height > 0);
            const dropArea = findChild(overflow, "applicationOverflowDropArea");
            verify(dropArea !== null);
            verify(dropArea.width > 0 && dropArea.height > 0);
            verify(overflow.popup.width < 350); // compact application grid, not the official system popup
            mouseMove(root, 0, 0);
            tryCompare(overflow, "expanded", false, 2000);
        }

        function test_fiveColumnsAndPanelIconSize() {
            overflow.expanded = true;
            tryCompare(overflow.popup, "visible", true);
            tryVerify(() => Math.floor(overflow.layout.width / overflow.layout.cellWidth) === 5);
            tryCompare(sizingProbe.iconContainer, "width", root.itemSize);
            tryCompare(sizingProbe.iconContainer, "height", root.itemSize);
            const smallCell = overflow.layout.cellWidth;

            root.itemSize = 32;
            tryCompare(sizingProbe.iconContainer, "width", root.itemSize);
            tryCompare(sizingProbe.iconContainer, "height", root.itemSize);
            tryVerify(() => overflow.layout.cellWidth > smallCell);
            tryVerify(() => Math.floor(overflow.layout.width / overflow.layout.cellWidth) === 5);
        }

        function test_clickToggle() {
            mouseClick(overflow.button, overflow.button.width / 2, overflow.button.height / 2);
            tryCompare(overflow, "expanded", true);
            mouseClick(overflow.button, overflow.button.width / 2, overflow.button.height / 2);
            tryCompare(overflow, "expanded", false);
        }
    }
}
