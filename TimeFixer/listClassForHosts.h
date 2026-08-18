#pragma once

#include <QObject>
#include "TcpClient.h"
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


private:
	QList <QPair<QString, QString>>hostArr;
	int counterHost = 0;
	TcpClient* host;
};

