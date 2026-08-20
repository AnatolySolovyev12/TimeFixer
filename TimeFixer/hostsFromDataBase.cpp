#include "hostsFromDataBase.h"

hostsFromDataBase::hostsFromDataBase(QObject *parent)
	: QObject(parent)
{}

hostsFromDataBase::~hostsFromDataBase()
{}


void hostsFromDataBase::pushHostInArr(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer)
{
	hostsArr.push_back(HostStruct{ name, ipPort, CSD, networkAddress, time, serial, softVer });
}


void hostsFromDataBase::showArray()
{
	qDebug() << "Count of hosts: "  <<  hostsArr.length() << '\n';

	int counter = 1;
	
	for (auto& val : hostsArr)
	{
		std::cout << counter++ << " - " << val.s_name.toStdString() <<  "   " << val.s_ipPort.toStdString() << "   " << val.s_CSD.toStdString() << "   " << val.s_networkAddress.toStdString() << "   " << val.s_time.toStdString() << "   " << val.s_serial.toStdString() << "   " << val.s_softVer.toStdString() << '\n';
	}
}



void hostsFromDataBase::makeClients()
{



}