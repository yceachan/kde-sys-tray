// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
#include "applicationtraycontroller.h"
#include "systemtraymodel.h"
#include "systemtraysettings.h"

ApplicationTrayController::ApplicationTrayController(QAbstractItemModel *model, SystemTraySettings *settings, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_settings(settings)
{
}

bool ApplicationTrayController::moveApplication(const QString &itemId, bool hidden, const QString &targetId, bool after)
{
    QStringList liveApplications;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        const auto index = m_model->index(row, 0);
        if (index.data(static_cast<int>(BaseModel::BaseRole::ItemType)).toString() == QLatin1String("StatusNotifier")) {
            liveApplications.append(index.data(static_cast<int>(BaseModel::BaseRole::ItemId)).toString());
        }
    }
    // Entries can vanish during a native drag; failed drops leave settings intact.
    if (itemId.isEmpty() || !liveApplications.contains(itemId)
        || (!targetId.isEmpty() && !liveApplications.contains(targetId))) {
        return false;
    }
    if (itemId == targetId) {
        return true;
    }

    auto order = m_settings->applicationOrder();
    for (const auto &id : liveApplications) {
        if (!order.contains(id)) {
            order.append(id);
        }
    }
    // Retain offline IDs so a restarted application returns to its saved place.
    order.removeAll(itemId);
    const auto target = targetId.isEmpty() ? order.size() : order.indexOf(targetId) + (after ? 1 : 0);
    order.insert(target, itemId);
    m_settings->setApplicationPlacement(itemId, hidden, order);
    return true;
}

#include "moc_applicationtraycontroller.cpp"
