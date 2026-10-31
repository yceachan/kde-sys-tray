/*
    SPDX-FileCopyrightText: 2020 Konrad Materka <materka@gmail.com>

    SPDX-FileCopyrightText: 2026 yceachan
    Modified 2026-10-31: Windows-style application overflow integration.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "systemtraysettings.h"

#include <KConfigLoader>

static const QString KNOWN_ITEMS_KEY = QStringLiteral("knownItems");
static const QString EXTRA_ITEMS_KEY = QStringLiteral("extraItems");
static const QString SHOW_ALL_ITEMS_KEY = QStringLiteral("showAllItems");
static const QString SHOWN_ITEMS_KEY = QStringLiteral("shownItems");
static const QString HIDDEN_ITEMS_KEY = QStringLiteral("hiddenItems");
static const QString DISABLED_SNIS_KEY = QStringLiteral("disabledStatusNotifiers");

SystemTraySettings::SystemTraySettings(KConfigLoader *config, QObject *parent)
    : QObject(parent)
    , config(config)
{
    connect(config, &KConfigLoader::configChanged, this, [this]() {
        if (!updatingConfigValue) {
            loadConfig();
        }
    });

    loadConfig();
}

bool SystemTraySettings::isKnownPlugin(const QString &pluginId)
{
    return m_knownItems.contains(pluginId);
}

const QStringList SystemTraySettings::knownPlugins() const
{
    return m_knownItems;
}

void SystemTraySettings::addKnownPlugin(const QString &pluginId)
{
    m_knownItems << pluginId;
    writeConfigValue(KNOWN_ITEMS_KEY, m_knownItems);
}

void SystemTraySettings::removeKnownPlugin(const QString &pluginId)
{
    m_knownItems.removeAll(pluginId);
    writeConfigValue(KNOWN_ITEMS_KEY, m_knownItems);
}

bool SystemTraySettings::isEnabledPlugin(const QString &pluginId) const
{
    return m_extraItems.contains(pluginId);
}

const QStringList SystemTraySettings::enabledPlugins() const
{
    return m_extraItems;
}

void SystemTraySettings::addEnabledPlugin(const QString &pluginId)
{
    m_extraItems << pluginId;
    writeConfigValue(EXTRA_ITEMS_KEY, m_extraItems);
    Q_EMIT enabledPluginsChanged({pluginId}, {});
}

void SystemTraySettings::removeEnabledPlugin(const QString &pluginId)
{
    m_extraItems.removeAll(pluginId);
    writeConfigValue(EXTRA_ITEMS_KEY, m_extraItems);
    Q_EMIT enabledPluginsChanged({}, {pluginId});
}

bool SystemTraySettings::isDisabledStatusNotifier(const QString &pluginId)
{
    return m_disabledStatusNotifiers.contains(pluginId);
}

bool SystemTraySettings::isShowAllItems() const
{
    return config->property(SHOW_ALL_ITEMS_KEY).toBool();
}

const QStringList SystemTraySettings::shownItems() const
{
    return config->property(SHOWN_ITEMS_KEY).toStringList();
}

const QStringList SystemTraySettings::hiddenItems() const
{
    return config->property(HIDDEN_ITEMS_KEY).toStringList();
}

QStringList SystemTraySettings::applicationOrder() const
{
    return config->property(QStringLiteral("applicationOrder")).toStringList();
}

void SystemTraySettings::setApplicationPlacement(const QString &itemId, bool hidden, const QStringList &order)
{
    auto shown = shownItems();
    auto folded = hiddenItems();
    shown.removeAll(itemId);
    folded.removeAll(itemId);
    if (hidden) {
        folded.append(itemId);
    } else {
        shown.append(itemId);
    }
    writeConfigValues({{SHOWN_ITEMS_KEY, shown}, {HIDDEN_ITEMS_KEY, folded}, {QStringLiteral("applicationOrder"), order}});
}

void SystemTraySettings::cleanupPlugin(const QString &pluginId)
{
    removeKnownPlugin(pluginId);
    removeEnabledPlugin(pluginId);

    QStringList shown = shownItems();
    shown.removeAll(pluginId);
    writeConfigValue(SHOWN_ITEMS_KEY, shown);

    QStringList hidden = hiddenItems();
    hidden.removeAll(pluginId);
    writeConfigValue(HIDDEN_ITEMS_KEY, hidden);
}

void SystemTraySettings::loadConfig()
{
    if (!config) {
        return;
    }
    config->load();

    m_knownItems = config->property(KNOWN_ITEMS_KEY).toStringList();

    QStringList extraItems = config->property(EXTRA_ITEMS_KEY).toStringList();
    if (extraItems != m_extraItems) {
        QStringList extraItemsOld = m_extraItems;
        m_extraItems = extraItems;
        notifyAboutChangedEnabledPlugins(extraItemsOld, m_extraItems);
    }
    m_disabledStatusNotifiers = config->property(DISABLED_SNIS_KEY).toStringList();

    Q_EMIT configurationChanged();
}

void SystemTraySettings::writeConfigValue(const QString &key, const QVariant &value)
{
    writeConfigValues({{key, value}});
}

void SystemTraySettings::writeConfigValues(const QVariantMap &values)
{
    if (!config) {
        return;
    }

    updatingConfigValue = true;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        auto *item = config->findItemByName(it.key());
        Q_ASSERT(item);
        item->setWriteFlags(KConfigBase::Notify);
        item->setProperty(it.value());
    }
    config->save();
    config->read();
    updatingConfigValue = false;
    Q_EMIT configurationChanged();
}

void SystemTraySettings::notifyAboutChangedEnabledPlugins(const QStringList &enabledPluginsOld, const QStringList &enabledPluginsNew)
{
    QStringList newlyEnabled;
    QStringList newlyDisabled;

    for (const QString &pluginId : enabledPluginsOld) {
        if (!enabledPluginsNew.contains(pluginId)) {
            newlyDisabled << pluginId;
        }
    }

    for (const QString &pluginId : enabledPluginsNew) {
        if (!enabledPluginsOld.contains(pluginId)) {
            newlyEnabled << pluginId;
        }
    }

    Q_EMIT enabledPluginsChanged(newlyEnabled, newlyDisabled);
}

#include "moc_systemtraysettings.cpp"
