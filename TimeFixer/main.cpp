#include <QtCore/QCoreApplication>
#include <qapplication.h>
#include <listClassForHosts.h>
#include "dataBaseClass.h"
#include "hostsFromDataBase.h"
#include <Windows.h>
#include <clocale>
#include <iostream>
#include <qtimer.h>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include <QInputDialog>



QTimer* regularTimer = nullptr;
QSystemTrayIcon* trayIcon = nullptr;
listClassForHosts* hostsList = nullptr;
hostsFromDataBase* hostsDataBase = nullptr;
dataBaseClass* dBclass = nullptr;



void iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::ActivationReason::DoubleClick) // требуется корректировка вывода часов переведённых в сутки или в часах свыше 24
	{
		trayIcon->showMessage("Next start:",  "hostsDataBase: " + QTime::currentTime().addSecs(regularTimer->remainingTime() / 1000).toString() + '\n' + "listClassForHosts: " + hostsList->returnTimeToNextCycle(), QSystemTrayIcon::Information, 5000);
	}
}



void cmdOpen()
{
	AllocConsole(); // Создаем консоль и присоединяем к ней текущий процесс
	FILE* stream; // то через что перенаправляем поток. Без данной переменной приложение будет крашится
	freopen_s(&stream, "CONOUT$", "w", stdout); // Перенаправляем стандартный вывод в CONOUT$
	freopen_s(&stream, "CONOUT$", "w", stderr); // Перенаправляем стандартный вывод ошибок в CONOUT$
}



void cmdClose()
{
	qDebug() << "\nProgramm disconnect from console.";

	FreeConsole(); // Отделяем процесс от cmd. После cmd закрываем руками.
}



void resetTimerForDbHosts()
{
	qDebug() << "\nTimer for DB Hosts was reset.";
	regularTimer->start(); // 86400000 - сутки, 21600000 - 6 часов
	hostsDataBase->clearAllArr();
	dBclass->getDeviceParams();
}



void resetTimerForListHosts()
{
	qDebug() << "\nTimer for List Hosts was reset.";
	hostsList->restartCycleFunc();
}



void setMaxSecondsM2M()
{
	bool ok = true;
	int maxSeconds = QInputDialog::getInt(nullptr, "Set value", "Set Max seconds for start update time:", 320000, 60, 1000000, 10, &ok);

	qDebug() << "Was set maximum value for diff between current time and Time M2M =" << maxSeconds;
}



int main(int argc, char* argv[])
{
	SetConsoleCP(65001);        // UTF‑8 вход
	SetConsoleOutputCP(65001);  // UTF‑8 вывод
	setlocale(LC_ALL, "ru_RU.UTF-8");

	regularTimer = new QTimer();

	QApplication app(argc, argv); // QCoreApplication - не используем если используется QWidget

	app.setQuitOnLastWindowClosed(false); // закрытие QInputDialog без основного окна закроет прилоржение если данный пункт будет true.

	trayIcon = new QSystemTrayIcon();
	trayIcon->setIcon(QIcon(QCoreApplication::applicationDirPath() + "\\icon.png"));

	QMenu* menu = new QMenu();
	QAction* restoreActionOpenCLI = menu->addAction("CMD open and connect");
	QAction* restoreActionHideCLI = menu->addAction("CMD disconnect");
	QAction* restoreActionRestartDbHosts = menu->addAction("Restart updateTime for DB Hosts");
	QAction* restoreActionRestartListHosts = menu->addAction("Restart updateTime for list Hosts");
	QAction* restoreSetMaxSecondsM2M = menu->addAction("Set Max second for M2M");
	QAction* quitAction = menu->addAction("Exit");

	trayIcon->setContextMenu(menu);
	trayIcon->setVisible(true);

	QObject::connect(restoreActionOpenCLI, &QAction::triggered, &cmdOpen);
	QObject::connect(restoreActionHideCLI, &QAction::triggered, &cmdClose);
	QObject::connect(restoreActionRestartDbHosts, &QAction::triggered, &resetTimerForDbHosts);
	QObject::connect(restoreActionRestartListHosts, &QAction::triggered, &resetTimerForListHosts);
	QObject::connect(restoreSetMaxSecondsM2M, &QAction::triggered,[]() {setMaxSecondsM2M();});
	QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
	QObject::connect(trayIcon, &QSystemTrayIcon::activated, &iconActivated);
	
	hostsList = new listClassForHosts(nullptr);
	hostsDataBase = new hostsFromDataBase(nullptr);
	dBclass = new dataBaseClass(nullptr);

	QObject::connect(dBclass, &dataBaseClass::deviceParams, hostsDataBase, &hostsFromDataBase::pushHostInArr);
	QObject::connect(dBclass, &dataBaseClass::showArray, hostsDataBase, &hostsFromDataBase::showArray);

	hostsDataBase->clearAllArr();
	dBclass->getDeviceParams();

	// Регулярные запуски

	regularTimer->start(23000000); // 86400000 - сутки, 21600000 - 6 часов

	QObject::connect(regularTimer, &QTimer::timeout, [&]() {

		hostsDataBase->clearAllArr();
		dBclass->getDeviceParams();
		
		});
		
	return app.exec();
}