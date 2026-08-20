#include <QtCore/QCoreApplication>
#include <listClassForHosts.h>
#include "dataBaseCLass.h"
#include "hostsFromDataBase.h"

#include <Windows.h>
#include <clocale>
#include <iostream>



int main(int argc, char* argv[])
{
	SetConsoleCP(65001);        // UTF‑8 вход
	SetConsoleOutputCP(65001);  // UTF‑8 вывод
	setlocale(LC_ALL, "ru_RU.UTF-8");

	QCoreApplication app(argc, argv);

	
	//listClassForHosts* hostsList = new listClassForHosts(nullptr);
	
	hostsFromDataBase* hostsDataBase = new hostsFromDataBase(nullptr);
	dataBaseCLass* dBclass = new dataBaseCLass(nullptr);

	QObject::connect(dBclass, &dataBaseCLass::deviceParams, hostsDataBase, &hostsFromDataBase::pushHostInArr);
	QObject::connect(dBclass, &dataBaseCLass::showArray, hostsDataBase, &hostsFromDataBase::showArray);

	//dBclass->getDeviceParams();

	return app.exec();
}