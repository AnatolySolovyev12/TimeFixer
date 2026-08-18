#pragma once

#include <QObject>


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
		QString s_networkAddress;
		QString s_time;
		QString s_serial;
		QString s_softVer;
	};

	void pushHostInArr(QString name, QString ipPort, QString networkAddress, QString time, QString serial, QString softVer);

private:

	QList<HostStruct>hostsArr;
};

