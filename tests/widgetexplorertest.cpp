// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
#include <QAbstractItemModel>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTest>
#include <memory>

using namespace Qt::StringLiterals;

class WidgetExplorerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void trayIsSupported();
};

void WidgetExplorerTest::trayIsSupported()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQml
        import org.kde.plasma.private.shell
        WidgetExplorer {
            Component.onCompleted: application = ""
        }
    )", QUrl());
    QTRY_VERIFY_WITH_TIMEOUT(component.status() != QQmlComponent::Loading, 5000);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> explorer(component.create());
    QVERIFY2(explorer != nullptr, qPrintable(component.errorString()));
    auto *model = qobject_cast<QAbstractItemModel *>(explorer->property("widgetsModel").value<QObject *>());
    QVERIFY(model);
    const auto roles = model->roleNames();
    const int pluginRole = roles.key("pluginName", -1);
    const int supportedRole = roles.key("isSupported", -1);
    const int apiRole = roles.key("apiVersion", -1);
    QVERIFY(pluginRole >= 0 && supportedRole >= 0 && apiRole >= 0);

    for (int row = 0; row < model->rowCount(); ++row) {
        const auto index = model->index(row, 0);
        if (index.data(pluginRole).toString() == u"local.plasma.windowstray") {
            qInfo() << "Widget Explorer API:" << index.data(apiRole).toString()
                    << "supported:" << index.data(supportedRole).toBool();
            QVERIFY2(index.data(supportedRole).toBool(), "Plasma Widget Explorer labels the tray as unsupported");
            return;
        }
    }
    QFAIL("Tray was not discovered by Plasma Widget Explorer");
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    WidgetExplorerTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "widgetexplorertest.moc"
