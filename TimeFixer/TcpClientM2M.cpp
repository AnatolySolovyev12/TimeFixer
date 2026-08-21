#include "TcpClientM2M.h"

TcpClientM2M::TcpClientM2M(QObject* parent) : QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientM2M::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientM2M::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientM2M::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientM2M::onErrorOccurred);

	myTimer = new QTimer();
	connect(myTimer, &QTimer::timeout, this, &TcpClientM2M::exchangeFromTimer);

}



TcpClientM2M::~TcpClientM2M()
{
	if (socket->isOpen()) {
		socket->close();
	}
}



void TcpClientM2M::connectToSavedHost()
{

	if (reConnectCounter >= 3)
	{
		counterForResend = 0;
		reConnectCounter = 0;
		stopConnectionWithHost();
	}
	else
	{
		if (socket->state() != QAbstractSocket::ConnectedState)
		{
			qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try connect to (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString() << ':' << m_port;

			reConnectCounter++;

			if (socket->state() == QAbstractSocket::UnconnectedState) {
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}
			else {
				socket->abort();
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}
		}
	}
}



void TcpClientM2M::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClientM2M::sendMessage(const QByteArray& message)
{
	if (socket->state() == QTcpSocket::ConnectedState)
	{
		socket->write(message);
		QString temp = '(' + QString::number(counterForResend) + ") >> ";
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "TX " + temp << message.toHex();
	}
	else
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Not connected to host.";
}



void TcpClientM2M::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeM2M();
}



void TcpClientM2M::onDisconnected()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientM2M::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX << " << data.toHex();


	/*
	if (data.toHex().length() < 35 && (serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]102" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109") && counterForResend >= 2)
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		getDaily();
		return;
	}
	*/


	if (counterForResend == 3) ////////////////// тут нужны правки
	{
		QString dateTaime = data.toHex();

		checkDateTimeFromDevice(dateTaime);
	}


	myTimer->stop();
	counterForResend++;
	reTransmitQuery = 0;

	changeTimeM2M();
}

void TcpClientM2M::exchangeFromTimer()
{
	++reTransmitQuery;

	changeTimeM2M();
}

void TcpClientM2M::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	m_ip = "";
	m_port = 0;
	counterForResend = 0;
	reTransmitQuery = 0;
	reConnectCounter = 0;
	emit finish();
}



void TcpClientM2M::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	socket->abort();
}



void TcpClientM2M::changeTimeM2M()
{
	if (counterForResend != 6)
	{
		QTimer::singleShot(50, [this]() {

			if (counterForResend == 0)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA02102214193A585818014050207EE060207EE0704000000070804000000074EE97E")));
			}

			if (counterForResend == 1)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA0450221411095BFE6E6006036A1090607608574050801018A0207808B0760857405080201AC0A80083030303030303030BE10040E01000000065F1F0400621E5DFFFF114C7E")));
			}

			if (counterForResend == 2)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141321BA2E6E600C001C100010000000201FF02004F267E")));
			}

			if (counterForResend == 3)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141763BA6E6E600C001C100080000010000FF0200601A7E")));// запрашиваем текущее время устройства
			}

			if (counterForResend == 4)
			{
				qDebug() << "codeForCorrect " << codeForCorrect;


				if (codeForCorrect != 9)
				{
					counterForResend--;
					counterForResend--;
				}
				else
					counterForResend++;

				if (codeForCorrect == 0)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FC7C3CE97E"))); //+900
				}
				else if (codeForCorrect == 1)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF06011003843B6D7E"))); //-900
				}
				else if (codeForCorrect == 2)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF06011000D55F047E"))); //-213
				}
				else if (codeForCorrect == 3)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FF1C52A07E"))); //+228
				}
				else if (codeForCorrect == 4)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110003367867E"))); //-51
				}
				else if (codeForCorrect == 5)
				{
					sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FF1C52A07E"))); //+38
				}
			}

			if (counterForResend == 5)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA008022141535C727E"))); // завершение при коррект
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 6;
			}

			myTimer->start(20000);
			});
	}
	else
	{
		myTimer->stop();
		socket->close();

		m_ip = "";
		m_port = 0;
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finish();
	}
}



void TcpClientM2M::checkDateTimeFromDevice(QString rxString)
{
	QString temp = rxString;

	qDebug() << '\n' << "QString in checkDateTimeFromDevice = " + temp;

	temp.chop(14);

	QString dateTime = temp.sliced(36);

	QString year = dateTime;
	year.chop(12);

	QString month = dateTime.sliced(4);
	month.chop(10);

	QString day = dateTime.sliced(6);
	day.chop(8);

	QString hour = dateTime.sliced(10);
	hour.chop(4);

	QString minute = dateTime.sliced(12);
	minute.chop(2);

	QString second = dateTime.sliced(14);

	qDebug() << dateTime;
	qDebug() << year;
	qDebug() << month;
	qDebug() << day;
	qDebug() << hour;
	qDebug() << minute;
	qDebug() << second;

	bool ok;
	qDebug() << QString::number(year.toUInt(&ok, 16)) + '\-' + QString::number(month.toUInt(&ok, 16)) + '\-' + QString::number(day.toUInt(&ok, 16)) + "   " + QString::number(hour.toUInt(&ok, 16)) + '\:' + QString::number(minute.toUInt(&ok, 16)) + '\:' + QString::number(second.toUInt(&ok, 16));

	QString fullDate = QString::number(year.toUInt(&ok, 16)) + '\-'
		+ (QString::number(month.toUInt(&ok, 16)).length() != 1 ? QString::number(month.toUInt(&ok, 16)) : ("0" + QString::number(month.toUInt(&ok, 16)))) + '\-'
		+ (QString::number(day.toUInt(&ok, 16)).length() != 1 ? QString::number(day.toUInt(&ok, 16)) : ("0" + QString::number(day.toUInt(&ok, 16))));

	qDebug() << "fullDate - " << QDate::fromString(fullDate, "yyyy-MM-dd").isValid() << fullDate;
	qDebug() << "CurrDate - " << QDate::currentDate().isValid() << QDate::currentDate();


	QString fullTime = (QString::number(hour.toUInt(&ok, 16)).length() != 1 ? QString::number(hour.toUInt(&ok, 16)) : ("0" + QString::number(hour.toUInt(&ok, 16)))) + '\:'
		+ (QString::number(minute.toUInt(&ok, 16)).length() != 1 ? QString::number(minute.toUInt(&ok, 16)) : ("0" + QString::number(minute.toUInt(&ok, 16)))) + '\:'
		+ (QString::number(second.toUInt(&ok, 16)).length() != 1 ? QString::number(second.toUInt(&ok, 16)) : ("0" + QString::number(second.toUInt(&ok, 16))));

	qDebug() << "FullTime - " << QTime::fromString(fullTime).isValid() << QTime::fromString(fullTime);
	qDebug() << "CurrTime - " << QTime::currentTime().isValid() << QTime::currentTime();

	qDebug() << "Date difference = " + QString::number(QDate::fromString(fullDate, "yyyy-MM-dd").daysTo(QDate::currentDate())) + " = " + QString::number(QDate::fromString(fullDate, "yyyy-MM-dd").daysTo(QDate::currentDate()) * 86400);

	qDebug() << "Time difference = " << QTime::fromString(fullTime).secsTo(QTime::currentTime());

	qDebug() << "Full difference = " << (QDate::fromString(fullDate, "yyyy-MM-dd").daysTo(QDate::currentDate()) * 86400) + QTime::fromString(fullTime).secsTo(QTime::currentTime());

	long seconds = (QDate::fromString(fullDate, "yyyy-MM-dd").daysTo(QDate::currentDate()) * 86400) + QTime::fromString(fullTime).secsTo(QTime::currentTime());

	if (!QTime::fromString(fullTime).isValid() || !QDate::fromString(fullDate, "yyyy-MM-dd").isValid())
	{
		codeForCorrect = 9;
		return;
	}

	if (abs(seconds) <= 50000)
	{
		if (abs(seconds) >= 900)
		{
			if (seconds < 0) codeForCorrect = 0;
			if (seconds > 0) codeForCorrect = 1;
		}
		else if (abs(seconds) < 900)
		{
			if (abs(seconds) > 200)
			{
				if (seconds > 0) codeForCorrect = 2;
				if (seconds < 0) codeForCorrect = 3;
			}
			else
			{
				if (abs(seconds) > 50)
				{
					if (seconds < 0) codeForCorrect = 5;
					if (seconds > 0) codeForCorrect = 4;
				}
				else
					codeForCorrect = 9;
			}
		}
		else
			codeForCorrect = 9;
	}
	else
		codeForCorrect = 9;
}
