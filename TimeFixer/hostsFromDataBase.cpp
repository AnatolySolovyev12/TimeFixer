#include "hostsFromDataBase.h"

hostsFromDataBase::hostsFromDataBase(QObject* parent)
	: QObject(parent)
{
	//TcpClientM2M* test = new TcpClientM2M(); ///////

	//test->startConnectToHost("172.16.53.243", "8888");
}

hostsFromDataBase::~hostsFromDataBase()
{
}


void hostsFromDataBase::pushHostInArr(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer)
{
	if (name.contains("М2М") || name.contains("M2M") && softVer.contains("1."))
		hostsArr.push_back(HostStruct{ name, ipPort, CSD, networkAddress, time, serial, softVer });
}


void hostsFromDataBase::showArray()
{
	qDebug() << "Count of hosts: " << hostsArr.length() << '\n';

	int counter = 1;

	for (auto& val : hostsArr)
	{
		std::cout << counter++ << " - " << val.s_name.toStdString() << "   " << val.s_ipPort.toStdString() << "   " << val.s_CSD.toStdString() << "   " << val.s_networkAddress.toStdString() << "   " << val.s_time.toStdString() << "   " << val.s_serial.toStdString() << "   " << val.s_softVer.toStdString() << '\n';
	}

	makeClients();
}



void hostsFromDataBase::makeClients()
{
	for (auto& val : hostsArr)
	{
		QString fullAdress = val.s_ipPort;
		QString ipFromDbTelegram;
		QString portFromDbTelegram;
		bool portBool = false;;

		fullAdress.trimmed();
		fullAdress.remove(QRegularExpression("[\\r\\n]")); // убираем возможный Enter из адреса хоста

		for (auto& val : fullAdress)
		{
			if (val == ':')
			{
				portBool = true;
				continue;
			}

			if (!portBool)
				ipFromDbTelegram += val;
			else
				portFromDbTelegram += val;
		}

		TcpClientM2M* temp = new TcpClientM2M();

		connect(temp, &TcpClientM2M::finish, temp, [temp]() {
			delete temp;
			});

		clientArr.push_back(temp);

		temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
	}
}


