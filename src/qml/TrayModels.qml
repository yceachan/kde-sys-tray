// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
pragma ComponentBehavior: Bound

import QtQml
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.core as PlasmaCore

QtObject {
    id: models
    required property var source

    readonly property QtObject applications: KItemModels.KSortFilterProxyModel {
        filterRoleName: "itemType"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) === "StatusNotifier"
        Component.onCompleted: sourceModel = models.source
    }
    readonly property QtObject systems: KItemModels.KSortFilterProxyModel {
        filterRoleName: "itemType"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) !== "StatusNotifier"
        Component.onCompleted: sourceModel = models.source
    }
    readonly property QtObject visibleApplications: KItemModels.KSortFilterProxyModel {
        filterRoleName: "effectiveStatus"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) === PlasmaCore.Types.ActiveStatus
        Component.onCompleted: sourceModel = models.applications
    }
    readonly property QtObject hiddenApplications: KItemModels.KSortFilterProxyModel {
        filterRoleName: "effectiveStatus"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) === PlasmaCore.Types.PassiveStatus
        Component.onCompleted: sourceModel = models.applications
    }
    readonly property QtObject visibleSystems: KItemModels.KSortFilterProxyModel {
        filterRoleName: "effectiveStatus"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) === PlasmaCore.Types.ActiveStatus
        Component.onCompleted: sourceModel = models.systems
    }
    readonly property QtObject hiddenSystems: KItemModels.KSortFilterProxyModel {
        filterRoleName: "effectiveStatus"
        filterRowCallback: (row, parent) => sourceModel.data(sourceModel.index(row, 0, parent), filterRole) === PlasmaCore.Types.PassiveStatus
        Component.onCompleted: sourceModel = models.systems
    }
}
