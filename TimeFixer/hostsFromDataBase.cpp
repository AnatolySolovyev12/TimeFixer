#include "hostsFromDataBase.h"

hostsFromDataBase::hostsFromDataBase(QObject *parent)
	: QObject(parent)
{}

hostsFromDataBase::~hostsFromDataBase()
{}


void hostsFromDataBase::pushHostInArr(QString name, QString ipPort, QString networkAddress, QString time, QString serial, QString softVer)
{
	hostsArr.push_back(HostStruct{ name, ipPort, networkAddress, time, serial, softVer });
}