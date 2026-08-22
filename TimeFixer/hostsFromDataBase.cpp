#include "hostsFromDataBase.h"

hostsFromDataBase::hostsFromDataBase(QObject* parent)
	: QObject(parent)
{

}



hostsFromDataBase::~hostsFromDataBase()
{
}



void hostsFromDataBase::clearAllArr()
{
	qDebug() << "\n\n\nClear all arrays with old client\n\n\n";
	clienArrM2M.clear();
	clientArrART.clear();
	hostsArr.clear();
}



void hostsFromDataBase::pushHostInArr(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer)
{
	// M2M - варианты с кириллицей и латиницей
	if (name.contains("М2М") && softVer.contains("1.") || name.contains("M2M") && softVer.contains("1.") /*|| name.contains("МАЯК-301") || name.contains("ПСЧ-3АРТ")*/)
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

		if (val.s_name.contains("М2М") || val.s_name.contains("M2M"))
		{
			TcpClientM2M* temp = new TcpClientM2M();

			connect(temp, &TcpClientM2M::finish, temp, [temp]() {
				if (temp != nullptr)
					delete temp;
				});

			clienArrM2M.push_back(temp);

			temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
		}
		/*
		if (val.s_name.contains("МАЯК-301") || val.s_name.contains("ПСЧ-3АРТ"))
		{
			TcpClientArt* temp = new TcpClientArt();

			connect(temp, &TcpClientArt::finishART, temp, [temp]() {
				if (temp != nullptr)
					delete temp;
				});

			clientArrART.push_back(temp);

			temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
		}*/
	}
}


