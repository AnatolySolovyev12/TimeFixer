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
	clientArrMIR.clear();
}



void hostsFromDataBase::pushHostInArr(QString name, QString ipPort, QString CSD, QString networkAddress, QString time, QString serial, QString softVer)
{
	// M2M - варианты с кириллицей и латиницей
	if (serial.contains("101000") && softVer.contains("1.") || serial.contains("102000") && softVer.contains("1.") || serial.contains("103000") && softVer.contains("1.") 
		|| serial.contains("104000") && softVer.contains("1.") || serial.contains("106000") && softVer.contains("1.") || serial.contains("109000") && softVer.contains("1.") 

		|| name.contains("М2М") && softVer.contains("1.") || name.contains("M2M") && softVer.contains("1.") 
		
		|| name.contains("МАЯК-301") || name.contains("ПСЧ-3АРТ")

		|| name.contains("МИР") || name.contains("С-04") || name.contains("С-05") || name.contains("С-07") || name.contains("C-04") || name.contains("C-05") 
		|| name.contains("C-07") || name.contains("481687") || name.contains("478212") || name.contains("464363") || name.contains("479687") || name.contains("494591"))

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

		if (val.s_name.contains("М2М") || val.s_name.contains("M2M")
			|| val.s_serial.contains("101000") && val.s_softVer.contains("1.") || val.s_serial.contains("102000") && val.s_softVer.contains("1.") || val.s_serial.contains("103000") && val.s_softVer.contains("1.")
			|| val.s_serial.contains("104000") && val.s_softVer.contains("1.") || val.s_serial.contains("106000") && val.s_softVer.contains("1.") || val.s_serial.contains("109000") && val.s_softVer.contains("1."))
		{
			TcpClientM2M* temp = new TcpClientM2M();

			connect(temp, &TcpClientM2M::finishM2M, [temp]() {
				if (temp != nullptr)
				{
					disconnect(temp, nullptr, nullptr, nullptr);
					temp->stopConnectionWithHost();
					temp->deleteLater();
				}
				});

			clienArrM2M.push_back(temp);

			temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
		}
		
		if (val.s_name.contains("МАЯК-301") || val.s_name.contains("ПСЧ-3АРТ"))
		{
			TcpClientArt* temp = new TcpClientArt();

			connect(temp, &TcpClientArt::finishART, [temp]() {
				if (temp != nullptr)
				{
					disconnect(temp, nullptr, nullptr, nullptr);
					temp->stopConnectionWithHost();
					temp->deleteLater();
				}
				});

			clientArrART.push_back(temp);

			temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
		}

		if(val.s_name.contains("МИР") || val.s_name.contains("С-04") || val.s_name.contains("С-05") || val.s_name.contains("С-07") || val.s_name.contains("C-04") || val.s_name.contains("C-05") 
			|| val.s_name.contains("C-07") || val.s_name.contains("481687") || val.s_name.contains("478212") || val.s_name.contains("464363") || val.s_name.contains("479687") || val.s_name.contains("494591"))
		{
			TcpClientMIR* temp = new TcpClientMIR();

			connect(temp, &TcpClientMIR::finishMIR, [temp]() {
				if (temp != nullptr)
				{
					disconnect(temp, nullptr, nullptr, nullptr);
					temp->stopConnectionWithHost();
					temp->deleteLater();
				}
				});

			clientArrMIR.push_back(temp);

			temp->startConnectToHost(ipFromDbTelegram, portFromDbTelegram);
		}
	}
}