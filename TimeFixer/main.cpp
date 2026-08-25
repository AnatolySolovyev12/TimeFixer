#include <QtCore/QCoreApplication>
#include <qapplication.h>
#include <listClassForHosts.h>
#include "dataBaseCLass.h"
#include "hostsFromDataBase.h"
#include <Windows.h>
#include <clocale>
#include <iostream>
#include <qtimer.h>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMainWindow>





QSystemTrayIcon* trayIcon = nullptr;

void iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::ActivationReason::DoubleClick) // требуется корректировка вывода часов переведённых в сутки или в часах свыше 24
	{
		trayIcon->showMessage("Test title", "testtesttest", QSystemTrayIcon::Information, 5000);
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



int main(int argc, char* argv[])
{
	SetConsoleCP(65001);        // UTF‑8 вход
	SetConsoleOutputCP(65001);  // UTF‑8 вывод
	setlocale(LC_ALL, "ru_RU.UTF-8");

	QTimer* regularTimer = new QTimer();

	QApplication app(argc, argv); // QCoreApplication - не используем если используется QWidget

	trayIcon = new QSystemTrayIcon();
	trayIcon->setIcon(QIcon(QCoreApplication::applicationDirPath() + "\\icon.png"));

	QMenu* menu = new QMenu();
	QAction* restoreAction = menu->addAction("CMD open and connect");
	QAction* restoreActionHide = menu->addAction("CMD disconnect");
	QAction* quitAction = menu->addAction("Exit");

	trayIcon->setContextMenu(menu);
	trayIcon->setVisible(true);

	QObject::connect(restoreAction, &QAction::triggered, &cmdOpen);
	QObject::connect(restoreActionHide, &QAction::triggered, &cmdClose);
	QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
	QObject::connect(trayIcon, &QSystemTrayIcon::activated, &iconActivated);
	
	listClassForHosts* hostsList = new listClassForHosts(nullptr);
	hostsFromDataBase* hostsDataBase = new hostsFromDataBase(nullptr);
	dataBaseCLass* dBclass = new dataBaseCLass(nullptr);

	QObject::connect(dBclass, &dataBaseCLass::deviceParams, hostsDataBase, &hostsFromDataBase::pushHostInArr);
	QObject::connect(dBclass, &dataBaseCLass::showArray, hostsDataBase, &hostsFromDataBase::showArray);

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