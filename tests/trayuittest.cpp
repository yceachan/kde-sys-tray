// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
#include <QApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTest>

#include <KPackage/PackageLoader>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <PlasmaQuick/AppletQuickItem>

using namespace Qt::StringLiterals;

class TestCorona : public Plasma::Corona
{
public:
    TestCorona()
    {
        auto package = KPackage::PackageLoader::self()->loadPackage(u"Plasma/Shell"_s);
        package.setPath(u"org.kde.plasma.desktop"_s);
        setKPackage(package);
    }
    QRect screenGeometry(int) const override { return {0, 0, 1280, 1024}; }
};

class TrayUiTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void independentApplicationPopup();
};

void TrayUiTest::independentApplicationPopup()
{
    TestCorona corona;
    auto *tray = corona.createContainment(u"local.plasma.windowstray"_s);
    QVERIFY(tray);
    tray->setFormFactor(Plasma::Types::Horizontal);
    tray->setLocation(Plasma::Types::BottomEdge);
    auto *root = PlasmaQuick::AppletQuickItem::itemForApplet(tray);
    QVERIFY2(root, qPrintable(tray->launchErrorMessage()));

    QQuickWindow window;
    window.resize(1024, 32);
    root->setParentItem(window.contentItem());
    root->setSize({1024, 32});
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *overflow = root->findChild<QQuickItem *>(u"applicationOverflow"_s);
    auto *applications = root->findChild<QQuickItem *>(u"visibleApplicationsGrid"_s);
    auto *systems = root->findChild<QQuickItem *>(u"visibleSystemsGrid"_s);
    auto *systemArrow = root->findChild<QQuickItem *>(u"systemTrayExpander"_s);
    QVERIFY(overflow && applications && systems && systemArrow);
    QTRY_VERIFY(overflow->width() > 0 && overflow->height() > 0);
    QVERIFY(overflow->isVisible());
    // Layouts settle after the native window configure, and fractional scale
    // factors can round adjacent edges to neighboring device pixels.
    QTRY_VERIFY(overflow->mapToItem(root, QPointF()).x() + 1 >= applications->mapToItem(root, QPointF()).x() + applications->width());
    QTRY_VERIFY(systems->mapToItem(root, QPointF()).x() + 1 >= overflow->mapToItem(root, QPointF()).x() + overflow->width());

    auto *systemState = root->property("systemTrayState").value<QObject *>();
    auto *applicationPopup = overflow->property("popup").value<QObject *>();
    auto *systemPopup = root->findChild<QObject *>(u"popupWindow"_s);
    QVERIFY(systemState && applicationPopup && systemPopup);
    QVERIFY(applicationPopup != systemPopup);

    overflow->setProperty("expanded", true);
    QTRY_VERIFY(applicationPopup->property("visible").toBool());
    QVERIFY(!systemState->property("expanded").toBool());
    QVERIFY(!systemPopup->property("visible").toBool());

    systemState->setProperty("expanded", true);
    QTRY_VERIFY(systemPopup->property("visible").toBool());
    QTRY_VERIFY(!applicationPopup->property("visible").toBool());
    QVERIFY(!overflow->property("expanded").toBool());
    systemState->setProperty("expanded", false);
    root->setParentItem(nullptr);
}

int main(int argc, char **argv)
{
    QTemporaryDir config;
    qputenv("XDG_CONFIG_HOME", config.path().toUtf8());
    QApplication app(argc, argv);
    TrayUiTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "trayuittest.moc"
