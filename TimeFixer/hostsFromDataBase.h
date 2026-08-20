#pragma once

#include <QObject>
#include <qdebug.h>
#include <iostream>

class hostsFromDataBase : public QObject
{
	Q_OBJECT

public:
	hostsFromDataBase(QObject* parent);
	~hostsFromDataBase();

	struct HostStruct
	{
		QString s_name;
		QString s_ipPort;
		QString s_CSD;
		QString s_networkAddress;
		QString s_time;
		QString s_serial;
		QString s_softVer;
	};

	void pushHostInArr(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer);
	void showArray();
	void makeClients();

private:

	QList<HostStruct>hostsArr;
};

