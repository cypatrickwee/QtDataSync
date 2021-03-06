#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "tst.h"
using namespace QtDataSync;

class LocalStoreTest : public QObject
{
	Q_OBJECT

Q_SIGNALS:
	void unlock();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testCount_data();
	void testCount();
	void testKeys_data();
	void testKeys();
	void testLoadAll_data();
	void testLoadAll();
	void testLoad_data();
	void testLoad();
	void testSave_data();
	void testSave();
	void testRemove_data();
	void testRemove();
	void testSearch_data();
	void testSearch();
	void testIterate_data();
	void testIterate();

	void testLoadInto_data();
	void testLoadInto();

private:
	MockLocalStore *store;
	AsyncDataStore *async;
};

void LocalStoreTest::initTestCase()
{
#ifdef Q_OS_LINUX
	Q_ASSERT(qgetenv("LD_PRELOAD").contains("Qt5DataSync"));
#endif

	tst_init();

	Setup setup;
	mockSetup(setup);
	store = static_cast<MockLocalStore*>(setup.localStore());
	store->enabled = true;
	setup.create();

	async = new AsyncDataStore(this);
}

void LocalStoreTest::cleanupTestCase()
{
	delete async;
	Setup::removeSetup(Setup::DefaultSetup);
}

void LocalStoreTest::testCount_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<int>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("emptyData") << DataSet()
							   << 0
							   << false;
	QTest::newRow("simpleData") << generateDataJson(5, 55)
								<< 50
								<< false;
	QTest::newRow("invalidData") << DataSet()
								 << 0
								 << true;
}

void LocalStoreTest::testCount()
{
	QFETCH(DataSet, data);
	QFETCH(int, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->count<TestData>();
		auto res = task.result();
		QVERIFY(!shouldFail);
		QCOMPARE(res, result);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testKeys_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<QStringList>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("emptyData") << DataSet()
							   << QStringList()
							   << false;
	QTest::newRow("simpleData") << generateDataJson(10, 15)
								<< generateDataKeys(10, 15)
								<< false;
	QTest::newRow("invalidData") << DataSet()
								 << QStringList()
								 << true;
}

void LocalStoreTest::testKeys()
{
	QFETCH(DataSet, data);
	QFETCH(QStringList, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->keys<TestData>();
		auto res = task.result();
		QVERIFY(!shouldFail);
		QLISTCOMPARE(res, result);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testLoadAll_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<QList<TestData>>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("emptyData") << DataSet()
							   << QList<TestData>()
							   << false;
	QTest::newRow("simpleData") << generateDataJson(10, 20)
								<< generateData(10, 20)
								<< false;
	QTest::newRow("invalidData") << DataSet()
								 << QList<TestData>()
								 << true;
}

void LocalStoreTest::testLoadAll()
{
	QFETCH(DataSet, data);
	QFETCH(QList<TestData>, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->loadAll<TestData>();
		auto res = task.result();
		QVERIFY(!shouldFail);
		QLISTCOMPARE(res, result);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testLoad_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<int>("key");
	QTest::addColumn<TestData>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("simpleData") << generateDataJson(42, 43)
								<< 42
								<< generateData(42)
								<< false;
	QTest::newRow("missingData") << DataSet()
								 << 5
								 << TestData()
								 << true;
	QTest::newRow("invalidData") << generateDataJson(0, 1)
								 << 0
								 << generateData(0)
								 << true;
}

void LocalStoreTest::testLoad()
{
	QFETCH(DataSet, data);
	QFETCH(int, key);
	QFETCH(TestData, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->load<TestData>(key);
		auto res = task.result();
		QVERIFY(!shouldFail);
		QCOMPARE(res, result);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testSave_data()
{
	QTest::addColumn<TestData>("data");
	QTest::addColumn<DataSet>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("simpleData") << generateData(42)
								<< generateDataJson(42, 43)
								<< false;
	QTest::newRow("invalidData") << generateData(0)
								 << generateDataJson(0, 1)
								 << true;
}

void LocalStoreTest::testSave()
{
	QFETCH(TestData, data);
	QFETCH(DataSet, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore.clear();
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->save<TestData>(data);
		task.waitForFinished();
		QVERIFY(!shouldFail);

		store->mutex.lock();
		QCOMPARE(store->pseudoStore, result);
		store->mutex.unlock();
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testRemove_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<int>("key");
	QTest::addColumn<DataSet>("result");
	QTest::addColumn<bool>("didRemove");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("simpleData") << generateDataJson(11, 12)
								<< 11
								<< DataSet()
								<< true
								<< false;
	QTest::newRow("missingData") << DataSet()
								 << 77
								 << DataSet()
								 << false
								 << false;
	QTest::newRow("invalidData") << generateDataJson(0, 1)
								 << 0
								 << DataSet()
								 << false
								 << true;
}

void LocalStoreTest::testRemove()
{
	QFETCH(DataSet, data);
	QFETCH(int, key);
	QFETCH(DataSet, result);
	QFETCH(bool, didRemove);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->remove<TestData>(key);
		auto res = task.result();
		QVERIFY(!shouldFail);
		QCOMPARE(res, didRemove);

		store->mutex.lock();
		[&](){//catch return to still unlock
			QCOMPARE(store->pseudoStore, result);
		}();
		store->mutex.unlock();
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testSearch_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<QString>("query");
	QTest::addColumn<QList<TestData>>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("emptyData") << DataSet()
							   << QStringLiteral("3")
							   << QList<TestData>()
							   << false;
	QTest::newRow("simpleData") << generateDataJson(10, 40)
								<< QStringLiteral("2")
								<< (generateData(12, 13) + generateData(20, 30) + generateData(32, 33))
								<< false;
	QTest::newRow("invalidData") << DataSet()
								 << QStringLiteral("baum")
								 << QList<TestData>()
								 << true;
}

void LocalStoreTest::testSearch()
{
	QFETCH(DataSet, data);
	QFETCH(QString, query);
	QFETCH(QList<TestData>, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->search<TestData>(query);
		auto res = task.result();
		QVERIFY(!shouldFail);
		QLISTCOMPARE(res, result);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

void LocalStoreTest::testIterate_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<QList<TestData>>("result");
	QTest::addColumn<bool>("shouldFail");

	QTest::newRow("emptyData") << DataSet()
							   << QList<TestData>()
							   << false;
	QTest::newRow("simpleData") << generateDataJson(5, 55)
								<< generateData(5, 55)
								<< false;
	QTest::newRow("invalidData") << generateDataJson(10, 20)
								 << QList<TestData>()
								 << true;
}

void LocalStoreTest::testIterate()
{
	QFETCH(DataSet, data);
	QFETCH(QList<TestData>, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	QSignalSpy spy(this, &LocalStoreTest::unlock);

	async->iterate<TestData>([=, &result](TestData data){
		auto ok = false;

		[&](){
			QVERIFY(!shouldFail);
			QVERIFY(!result.isEmpty());
			for(auto i = 0; i < result.size(); i++) {
				if(data.id == result[i].id) {
					QCOMPARE(data, result[i]);
					result.takeAt(i);
					ok = true;
					break;
				}
			}
		}();

		if(!ok || result.isEmpty())
			emit unlock();

		return ok;
	}, [=](const QException &e){
		QVERIFY2(shouldFail, e.what());
		emit unlock();
	});

	if(data.isEmpty())
		QVERIFY(!spy.wait());
	else {
		QVERIFY(spy.wait());
		QVERIFY(result.isEmpty());
	}
}

void LocalStoreTest::testLoadInto_data()
{
	QTest::addColumn<DataSet>("data");
	QTest::addColumn<TestObject*>("target");
	QTest::addColumn<int>("key");
	QTest::addColumn<TestObject*>("result");
	QTest::addColumn<bool>("shouldFail");

	DataSet dataHash;
	dataHash.insert({"TestObject*", QString::number(42)}, generateDataJson(42));

	QTest::newRow("simpleData") << dataHash
								<< new TestObject(42, QString(), this)
								<< 42
								<< new TestObject(42, QString::number(42), this)
								<< false;
	QTest::newRow("missingData") << DataSet()
								 << new TestObject(this)
								 << 5
								 << new TestObject(this)
								 << true;
	QTest::newRow("invalidData") << dataHash
								 << new TestObject(13, QString(), this)
								 << 42
								 << new TestObject(this)
								 << true;
}

void LocalStoreTest::testLoadInto()
{
	QFETCH(DataSet, data);
	QFETCH(TestObject*, target);
	QFETCH(int, key);
	QFETCH(TestObject*, result);
	QFETCH(bool, shouldFail);

	store->mutex.lock();
	store->pseudoStore = data;
	store->failCount = shouldFail ? 1 : 0;
	store->mutex.unlock();

	try {
		auto task = async->loadInto<TestObject*>(key, target);
		auto res = task.result();
		QVERIFY(!shouldFail);
		QCOMPARE(res, target);
		QCOMPARE(target->id, result->id);
		QCOMPARE(target->text, result->text);
	} catch(QException &e) {
		QVERIFY2(shouldFail, e.what());
	}
}

QTEST_MAIN(LocalStoreTest)

#include "tst_localstore.moc"
