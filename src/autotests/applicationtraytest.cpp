// SPDX-FileCopyrightText: 2026 yceachan
// SPDX-License-Identifier: GPL-2.0-or-later
#include "../applicationtraycontroller.h"
#include "../sortedsystemtraymodel.h"
#include "../systemtraymodel.h"
#include "../systemtraysettings.h"

#include <KConfigLoader>
#include <QFile>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;

static constexpr int typeRole = static_cast<int>(BaseModel::BaseRole::ItemType);
static constexpr int idRole = static_cast<int>(BaseModel::BaseRole::ItemId);
static constexpr int categoryRole = static_cast<int>(BaseModel::BaseRole::Category);

class StatusProbe : public BaseModel
{
public:
    using BaseModel::BaseModel;
    using BaseModel::calculateEffectiveStatus;
    int rowCount(const QModelIndex & = {}) const override { return 0; }
    QVariant data(const QModelIndex &, int) const override { return {}; }
};

class ApplicationTrayTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void foldAndPin();
    void persistenceAndOfflineOrder();
    void rejectSystemAndMissingItems();
    void reorderApplicationsOnly();
    void visibilityBoundary();

private:
    void add(const QString &id, const QString &type, const QString &category);
    QStringList ids(const QAbstractItemModel &model) const;
    std::unique_ptr<QTemporaryDir> dir;
    std::unique_ptr<KConfigLoader> loader;
    std::unique_ptr<SystemTraySettings> settings;
    QStandardItemModel source;
};

void ApplicationTrayTest::init()
{
    source.clear();
    settings.reset();
    loader.reset();
    dir = std::make_unique<QTemporaryDir>();
    QVERIFY(dir->isValid());
    QFile schema(QFINDTESTDATA("../main.xml"));
    loader = std::make_unique<KConfigLoader>(dir->filePath(u"trayrc"_s), &schema);
    settings = std::make_unique<SystemTraySettings>(loader.get());
    add(u"app-a"_s, u"StatusNotifier"_s, u"ApplicationStatus"_s);
    add(u"network"_s, u"Plasmoid"_s, u"Hardware"_s);
    add(u"app-b"_s, u"StatusNotifier"_s, u"Hardware"_s);
    add(u"volume"_s, u"Plasmoid"_s, u"Hardware"_s);
    add(u"background"_s, u"BackgroundApp"_s, u"ApplicationStatus"_s);
}

void ApplicationTrayTest::add(const QString &id, const QString &type, const QString &category)
{
    auto *item = new QStandardItem(id);
    item->setData(id, idRole);
    item->setData(type, typeRole);
    item->setData(category, categoryRole);
    source.appendRow(item);
}

QStringList ApplicationTrayTest::ids(const QAbstractItemModel &model) const
{
    QStringList result;
    for (int row = 0; row < model.rowCount(); ++row) {
        result.append(model.index(row, 0).data(idRole).toString());
    }
    return result;
}

void ApplicationTrayTest::foldAndPin()
{
    ApplicationTrayController controller(&source, settings.get());
    QSignalSpy changes(settings.get(), &SystemTraySettings::configurationChanged);
    QVERIFY(controller.moveApplication(u"app-a"_s, true));
    QCOMPARE(settings->hiddenItems(), QStringList{u"app-a"_s});
    QVERIFY(!settings->shownItems().contains(u"app-a"_s));
    QCOMPARE(changes.count(), 1); // placement and ordering publish atomically

    QVERIFY(controller.moveApplication(u"app-a"_s, false, u"app-b"_s));
    QCOMPARE(settings->shownItems(), QStringList{u"app-a"_s});
    QVERIFY(!settings->hiddenItems().contains(u"app-a"_s));
    QCOMPARE(settings->applicationOrder(), (QStringList{u"app-a"_s, u"app-b"_s}));
}

void ApplicationTrayTest::persistenceAndOfflineOrder()
{
    ApplicationTrayController controller(&source, settings.get());
    QVERIFY(controller.moveApplication(u"app-b"_s, true, u"app-a"_s));
    source.removeRow(2); // application temporarily exits
    QVERIFY(controller.moveApplication(u"app-a"_s, false));
    QCOMPARE(settings->applicationOrder(), (QStringList{u"app-b"_s, u"app-a"_s}));

    QFile schema(QFINDTESTDATA("../main.xml"));
    KConfigLoader restored(dir->filePath(u"trayrc"_s), &schema);
    SystemTraySettings saved(&restored);
    QCOMPARE(saved.applicationOrder(), settings->applicationOrder());
    QCOMPARE(saved.hiddenItems(), QStringList{u"app-b"_s});
    QCOMPARE(saved.shownItems(), QStringList{u"app-a"_s});
}

void ApplicationTrayTest::rejectSystemAndMissingItems()
{
    ApplicationTrayController controller(&source, settings.get());
    QSignalSpy changes(settings.get(), &SystemTraySettings::configurationChanged);
    QVERIFY(!controller.moveApplication(u"network"_s, true));
    QVERIFY(!controller.moveApplication(u"background"_s, true));
    QVERIFY(!controller.moveApplication(u"app-a"_s, false, u"volume"_s));
    QVERIFY(!controller.moveApplication(u"exited-app"_s, false));
    QVERIFY(!controller.moveApplication(u"app-a"_s, false, u"exited-app"_s));
    QVERIFY(!controller.moveApplication(QString(), false));
    QVERIFY(controller.moveApplication(u"app-a"_s, false, u"app-a"_s));
    QCOMPARE(changes.count(), 0);
    QVERIFY(settings->applicationOrder().isEmpty());
}

void ApplicationTrayTest::reorderApplicationsOnly()
{
    SortedSystemTrayModel sorted(SortedSystemTrayModel::SortingType::SystemTray, nullptr, settings.get());
    sorted.setSourceModel(&source);
    const auto original = ids(sorted).mid(2);
    ApplicationTrayController controller(&sorted, settings.get());

    QVERIFY(controller.moveApplication(u"app-b"_s, false, u"app-a"_s));
    QCOMPARE(ids(sorted).first(), u"app-b"_s);
    QCOMPARE(ids(sorted).at(1), u"app-a"_s);
    QCOMPARE(ids(sorted).mid(2), original);

    QVERIFY(controller.moveApplication(u"app-b"_s, false, u"app-a"_s, true));
    QCOMPARE(ids(sorted).first(), u"app-a"_s);
    QCOMPARE(ids(sorted).at(1), u"app-b"_s);
    QCOMPARE(ids(sorted).mid(2), original);
}

void ApplicationTrayTest::visibilityBoundary()
{
    loader->findItemByName(u"showAllItems"_s)->setProperty(true);
    loader->save();
    ApplicationTrayController controller(&source, settings.get());
    StatusProbe probe(settings.get());

    QVERIFY(controller.moveApplication(u"app-a"_s, true));
    QCOMPARE(probe.calculateEffectiveStatus(true, Plasma::Types::ActiveStatus, u"app-a"_s, true), Plasma::Types::PassiveStatus);
    QCOMPARE(probe.calculateEffectiveStatus(true, Plasma::Types::ActiveStatus, u"network"_s), Plasma::Types::ActiveStatus);
    QCOMPARE(probe.calculateEffectiveStatus(false, Plasma::Types::ActiveStatus, u"app-a"_s, true), Plasma::Types::HiddenStatus);
    QCOMPARE(probe.calculateEffectiveStatus(true, Plasma::Types::HiddenStatus, u"app-a"_s, true), Plasma::Types::ActiveStatus);

    QVERIFY(controller.moveApplication(u"app-a"_s, false));
    QCOMPARE(probe.calculateEffectiveStatus(true, Plasma::Types::PassiveStatus, u"app-a"_s, true), Plasma::Types::ActiveStatus);
}

QTEST_MAIN(ApplicationTrayTest)
#include "applicationtraytest.moc"
