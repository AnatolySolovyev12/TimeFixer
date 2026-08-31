#pragma once

#include <QObject>
#include "TcpClientArt.h"
#include <qdatetime.h>
#include <QFile>
#include <QCoreApplication>
#include <qtimer.h>

class listClassForHosts  : public QObject
{
	Q_OBJECT

public:
	listClassForHosts(QObject *parent);
	bool readHostsFile(QList <QPair<QString, QString>>& temp);
	void initializeFunc();
	void switchToNextHost();
	QString returnTimeToNextCycle();
	void restartCycleFunc();

private:
	QList <QPair<QString, QString>>hostArr;
	int counterHost = 0;
	TcpClientArt* host;
	QString timeToNextCycle = "";
	QTimer* restartTimer = nullptr;


};

