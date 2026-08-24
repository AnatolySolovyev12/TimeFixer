#pragma once

#include <QObject>
#include <qdebug.h>
#pragma once

#include <iostream>
#include <qdatetime.h>
#include "TcpClientM2M.h"
#include "TcpClientArt.h"
#include "TcpClientMIR.h"
#include "QRegularExpression"

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
	void clearAllArr();

private:

	QList<HostStruct>hostsArr;
	QList<TcpClientM2M*>clienArrM2M;
	QList<TcpClientArt*>clientArrART;
	QList<TcpClientMIR*>clientArrMIR;
};

