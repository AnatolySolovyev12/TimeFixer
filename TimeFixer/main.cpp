#include <QtCore/QCoreApplication>
#include <listClassForHosts.h>
#include "dataBaseCLass.h"
#include "hostsFromDataBase.h"

#include <Windows.h>
#include <clocale>
#include <iostream>
#include <qtimer.h>



int main(int argc, char* argv[])
{
	SetConsoleCP(65001);        // UTF‑8 вход
	SetConsoleOutputCP(65001);  // UTF‑8 вывод
	setlocale(LC_ALL, "ru_RU.UTF-8");

	QTimer* regularTimer = new QTimer();

	QCoreApplication app(argc, argv);

	listClassForHosts* hostsList = new listClassForHosts(nullptr);
	
	hostsFromDataBase* hostsDataBase = new hostsFromDataBase(nullptr);
	dataBaseCLass* dBclass = new dataBaseCLass(nullptr);

	QObject::connect(dBclass, &dataBaseCLass::deviceParams, hostsDataBase, &hostsFromDataBase::pushHostInArr);
	QObject::connect(dBclass, &dataBaseCLass::showArray, hostsDataBase, &hostsFromDataBase::showArray);

	hostsDataBase->clearAllArr();
	dBclass->getDeviceParams();

	// Регулярные запуски

	regularTimer->start(23000000);

	QObject::connect(regularTimer, &QTimer::timeout, [&]() {

		hostsDataBase->clearAllArr();
		dBclass->getDeviceParams();

		});

	return app.exec();
}