#include "listClassForHosts.h"

listClassForHosts::listClassForHosts(QObject *parent)
	: QObject(parent)
{
	AttachConsole(ATTACH_PARENT_PROCESS);

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

	if (temp.length() > 0)
		return true;
	else 
		return false;
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
		QTimer::singleShot(20000000, [this]() // 86400000 - сутки, 21600000 - 6 часов
			{
				counterHost = 0;
				qDebug() << "\n\n\n" << "Restart All Session and start new session (" + QString::number(counterHost + 1) + '/' + QString::number(hostArr.length()) + "): " << hostArr[counterHost].first << "   " << hostArr[counterHost].second;
				host->startConnectToHost(hostArr[counterHost].first, hostArr[counterHost].second);
				timeToNextCycle = QTime::currentTime().addSecs(20000000 / 1000).toString();
			});

		timeToNextCycle = QTime::currentTime().addSecs(20000000 / 1000).toString();

		return;
	}
	else
	{
		qDebug() << "\n\n\n" << "Start new session (" + QString::number(counterHost + 1) + '/' + QString::number(hostArr.length()) + "): " << hostArr[counterHost].first << "   " << hostArr[counterHost].second;
		host->startConnectToHost(hostArr[counterHost].first, hostArr[counterHost].second);
		timeToNextCycle = "isNow";
	}
}



QString listClassForHosts::returnTimeToNextCycle()
{
	return timeToNextCycle;
}