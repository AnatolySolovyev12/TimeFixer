#pragma once

#include <QObject>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>


class dataBaseCLass  : public QObject
{
	Q_OBJECT

public:
	dataBaseCLass(QObject *parent);
	~dataBaseCLass();

	void connectDataBase();
	void getDeviceParams();

signals:
	void deviceParams(QString name, QString ipPort, QString networkAddress, QString time, QString serial, QString softVer);

private:
	QSqlDatabase mw_db;


	bool resultBool = false;
	QString odbcName = "DBEG";

	QString ipForTcp;
};