/*
    SPDX-FileCopyrightText: 2019 Konrad Materka <materka@gmail.com>

    SPDX-FileCopyrightText: 2026 yceachan
    Modified 2026-10-31: Windows-style application overflow integration.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sortedsystemtraymodel.h"
#include "debug.h"
#include "systemtraymodel.h"
#include "systemtraysettings.h"

#include <QList>

static const QList<QString> s_categoryOrder = {
    QStringLiteral("UnknownCategory"),
    QStringLiteral("ApplicationStatus"),
    QStringLiteral("Communications"),
    QStringLiteral("SystemServices"),
    QStringLiteral("Hardware"),
};

SortedSystemTrayModel::SortedSystemTrayModel(SortingType sorting, QObject *parent, SystemTraySettings *settings)
    : QSortFilterProxyModel(parent)
    , m_sorting(sorting)
    , m_settings(settings)
{
    setSortLocaleAware(true);
    if (settings) {
        connect(settings, &SystemTraySettings::configurationChanged, this, &SortedSystemTrayModel::invalidate);
    }
    sort(0);
}

bool SortedSystemTrayModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    switch (m_sorting) {
    case SortedSystemTrayModel::SortingType::ConfigurationPage:
        return lessThanConfigurationPage(left, right);
    case SortedSystemTrayModel::SortingType::SystemTray:
        return lessThanSystemTray(left, right);
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool SortedSystemTrayModel::lessThanConfigurationPage(const QModelIndex &left, const QModelIndex &right) const
{
    const int categoriesComparison = compareCategoriesAlphabetically(left, right);
    if (categoriesComparison == 0) {
        return QSortFilterProxyModel::lessThan(left, right);
    } else {
        return categoriesComparison < 0;
    }
}

bool SortedSystemTrayModel::lessThanSystemTray(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant itemIdLeft = left.data(static_cast<int>(BaseModel::BaseRole::ItemId));
    QVariant itemIdRight = right.data(static_cast<int>(BaseModel::BaseRole::ItemId));
    if (itemIdRight.toString() == QLatin1String("org.kde.plasma.notifications")) {
        // return false when at least right is "org.kde.plasma.notifications"
        return false;
    } else if (itemIdLeft.toString() == QLatin1String("org.kde.plasma.notifications")) {
        // return true when only left is "org.kde.plasma.notifications"
        return true;
    }

    // Keep applications in their own reorderable group. System widgets retain
    // the upstream relative order and cannot become application drop targets.
    const auto typeRole = static_cast<int>(BaseModel::BaseRole::ItemType);
    const bool leftApplication = left.data(typeRole).toString() == QLatin1String("StatusNotifier");
    const bool rightApplication = right.data(typeRole).toString() == QLatin1String("StatusNotifier");
    if (leftApplication != rightApplication) {
        return leftApplication;
    }
    if (leftApplication && m_settings) {
        const auto order = m_settings->applicationOrder();
        const auto leftIndex = order.indexOf(itemIdLeft.toString());
        const auto rightIndex = order.indexOf(itemIdRight.toString());
        if (leftIndex != rightIndex) {
            return rightIndex < 0 || (leftIndex >= 0 && leftIndex < rightIndex);
        }
    }

    const int categoriesComparison = compareCategoriesOrderly(left, right);
    if (categoriesComparison == 0) {
        return QSortFilterProxyModel::lessThan(left, right);
    } else {
        return categoriesComparison < 0;
    }
}

int SortedSystemTrayModel::compareCategoriesAlphabetically(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = left.data(static_cast<int>(BaseModel::BaseRole::Category));
    QString leftCategory = leftData.isNull() ? QStringLiteral("UnknownCategory") : leftData.toString();

    QVariant rightData = right.data(static_cast<int>(BaseModel::BaseRole::Category));
    QString rightCategory = rightData.isNull() ? QStringLiteral("UnknownCategory") : rightData.toString();

    return QString::localeAwareCompare(leftCategory, rightCategory);
}

int SortedSystemTrayModel::compareCategoriesOrderly(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = left.data(static_cast<int>(BaseModel::BaseRole::Category));
    QString leftCategory = leftData.isNull() ? QStringLiteral("UnknownCategory") : leftData.toString();

    QVariant rightData = right.data(static_cast<int>(BaseModel::BaseRole::Category));
    QString rightCategory = rightData.isNull() ? QStringLiteral("UnknownCategory") : rightData.toString();

    int leftIndex = s_categoryOrder.indexOf(leftCategory);
    if (leftIndex == -1) {
        leftIndex = s_categoryOrder.indexOf(QStringLiteral("UnknownCategory"));
    }

    int rightIndex = s_categoryOrder.indexOf(rightCategory);
    if (rightIndex == -1) {
        rightIndex = s_categoryOrder.indexOf(QStringLiteral("UnknownCategory"));
    }

    return leftIndex - rightIndex;
}

#include "moc_sortedsystemtraymodel.cpp"
