#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "sampledata.h"

#include <QTreeWidget>
#include <QWidget>
#include <asyncdatastore.h>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	explicit MainWidget(QWidget *parent = nullptr);
	~MainWidget();

public slots:
	void report(QtMsgType type, const QString &message);

private slots:
	void dataChanged(int metaTypeId, const QString &key, bool wasDeleted);

	void on_addButton_clicked();
	void on_deleteButton_clicked();

	void update(SampleData *data);
	void reload();

	void setup();

private:
	Ui::MainWidget *ui;

	QtDataSync::AsyncDataStore *store;
	QHash<int, QTreeWidgetItem*> items;
};

#endif // MAINWIDGET_H
