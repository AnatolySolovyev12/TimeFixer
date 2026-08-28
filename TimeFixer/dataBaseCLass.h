#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCoreApplication>
#include <Windows.h>
#include <QFile>

class dataBaseCLass  : public QObject
{
	Q_OBJECT

public:
	dataBaseCLass(QObject *parent);
	~dataBaseCLass();

	void connectDataBase();
	void getDeviceParams();
	QString removeSimbols(QString temp);
	bool readDataBaseFile();

signals:
	void deviceParams(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer);
	void showArray();

private:
	QSqlDatabase mw_db;
	QString host = "";
	QString dataBase = "";
	QString login = "";
	QString pass = "";
};