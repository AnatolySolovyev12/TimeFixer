#include "listClassForHosts.h"

listClassForHosts::listClassForHosts(QObject *parent)
	: QObject(parent)
{
	if (readHostsFile(hostArr))
		initializeFunc();
}



bool listClassForHosts::readHostsFile(QList <QPair<QString, QString>>& temp)
{
	QFile file(QCoreApplication::applicationDirPath() + "\\hosts.txt");

	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Don't find hosts file. Create file and try again";
		return false;
	}

	QTextStream out(&file);

	bool portBool = false;
	QString ip;
	QString port;
	QString* myLine = new QString();

	while (out.readLineInto(myLine, 0))
	{
		for (auto& val : *myLine)
		{
			if (val.isSpace())
			{
				portBool = true;
				continue;
			}

			if (!portBool)
			{
				ip += val;
			}
			else
			{
				port += val;
			}
		}

		temp.push_back(qMakePair(ip, port));
		ip.clear();
		port.clear();
		portBool = false;
	}

	delete myLine;
	myLine = nullptr;

	file.close();

	qDebug() << temp;
	qDebug() << "Count of Hosts = " << temp.length();

	return true;
}



void listClassForHosts::initializeFunc()
{
	host = new TcpClientArt();

	connect(host, &TcpClientArt::finishART, this, [this]() {

		++counterHost;
		QTimer::singleShot(1000, [this]() { switchToNextHost(); });

		});
	
	switchToNextHost();
}



void listClassForHosts::switchToNextHost()
{
	if (counterHost >= hostArr.length() || hostArr[counterHost].first == "" || hostArr[counterHost].second == "")
	{
		QTimer::singleShot(20000000, [this]()
			{
				counterHost = 0;
				qDebug() << "\n\n\n" << "Restart All Session and start new session (" + QString::number(counterHost + 1) + '/' + QString::number(hostArr.length()) + "): " << hostArr[counterHost].first << "   " << hostArr[counterHost].second;
				host->startConnectToHost(hostArr[counterHost].first, hostArr[counterHost].second);
			});

		return;
	}
	else
	{
		qDebug() << "\n\n\n" << "Start new session (" + QString::number(counterHost + 1) + '/' + QString::number(hostArr.length()) + "): " << hostArr[counterHost].first << "   " << hostArr[counterHost].second;
		host->startConnectToHost(hostArr[counterHost].first, hostArr[counterHost].second);
	}
}