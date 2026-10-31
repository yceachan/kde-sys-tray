// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <QObject>

class QAbstractItemModel;
class SystemTraySettings;

// The sole mutation boundary for drag-and-drop; validates against live entries.
class ApplicationTrayController : public QObject
{
    Q_OBJECT
public:
    ApplicationTrayController(QAbstractItemModel *model, SystemTraySettings *settings, QObject *parent = nullptr);
    Q_INVOKABLE bool moveApplication(const QString &itemId, bool hidden, const QString &targetId = QString(), bool after = false);

private:
    QAbstractItemModel *m_model;
    SystemTraySettings *m_settings;
};
