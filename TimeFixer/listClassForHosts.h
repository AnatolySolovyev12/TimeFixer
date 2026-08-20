#pragma once

#include <QObject>
#include "TcpClientArt.h"
#include <qdatetime.h>
#include <QFile>
#include <QCoreApplication>

class listClassForHosts  : public QObject
{
	Q_OBJECT

public:
	listClassForHosts(QObject *parent);
	bool readHostsFile(QList <QPair<QString, QString>>& temp);
	void initializeFunc();
	void switchToNextHost();


private:
	QList <QPair<QString, QString>>hostArr;
	int counterHost = 0;
	TcpClientArt* host;
};

