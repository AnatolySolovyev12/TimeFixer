#include "TcpClientMIR.h"

TcpClientMIR::TcpClientMIR(QObject* parent) : QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientMIR::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientMIR::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientMIR::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientMIR::onErrorOccurred);

	myTimer = new QTimer();
	connect(myTimer, &QTimer::timeout, this, &TcpClientMIR::exchangeFromTimer);

}



TcpClientMIR::~TcpClientMIR()
{
	if (socket->isOpen()) {
		socket->close();
	}
}



void TcpClientMIR::connectToSavedHost()
{

	if (reConnectCounter >= 3)
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishMIR();
	}
	else
	{
		if (socket->state() != QAbstractSocket::ConnectedState)
		{
			qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try connect to (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString() << ':' << m_port;

			reConnectCounter++;

			if (socket->state() == QAbstractSocket::UnconnectedState)
			{
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}
			else
			{
				socket->abort();
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}
		}
	}
}



void TcpClientMIR::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClientMIR::sendMessage(const QByteArray& message)
{
	if (socket->state() == QTcpSocket::ConnectedState)
	{
		socket->write(message);
		QString temp = '(' + QString::number(counterForResend) + ") >> ";
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "TX (" + m_ip + ')' + temp << message.toHex();
	}
	else
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Not connected to host.";
}



void TcpClientMIR::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeMIR();
}



void TcpClientMIR::onDisconnected()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientMIR::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX (" + m_ip + ") << " << data.toHex();

	myTimer->stop();
	counterForResend++;
	reTransmitQuery = 0;

	changeTimeMIR();
}



void TcpClientMIR::exchangeFromTimer()
{
	++reTransmitQuery;
	changeTimeMIR();
}



void TcpClientMIR::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	if (socket->errorString().contains("The remote host closed the connection"))
	{
		++reConnectCounter;
		connectToSavedHost();
		qDebug() << '\n' << "TcpClientMIR::onErrorOccurred() -> Socket not open. Try reconnect.";
		return;
	}
	else
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishMIR();
	}
}



void TcpClientMIR::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	if (socket->state() == QAbstractSocket::ConnectedState)
		socket->abort();
}



void TcpClientMIR::changeTimeMIR()
{
	if (counterForResend != 9)
	{
		QTimer::singleShot(50, [this]() {

			if (counterForResend == 0)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea00a0002feffc99384a57e")));
			}

			if (counterForResend == 1)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea0470002feffc9105efae6e6006036a1090607608574050801018a0207808b0760857405080201ac0a80083030303030303030be10040e01000000065f1f0400007e1fffff00d17e")));
			}

			if (counterForResend == 2)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea01c0002feffc93203fce6e600c0018100010000600100ff02008c6d7e")));
			}

			if (counterForResend == 3)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea03b0002feffc954a85ee6e600c003810400460000800200ff020000460000800201ff020000460000800202ff02000046000060030aff0200ae957e")));
			}

			if (counterForResend == 4)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea01c0002feffc97623f8e6e600c0018100080000010000ff020065d77e")));

			}

			if (counterForResend == 5)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea01c0002feffc99853f6e6e600c0018100080000010000ff020065d77e")));
			}

			if (counterForResend == 6)
			{
				QByteArray firstGeneral = QByteArray::fromHex("a02a0002feffc9bac47ae6e600c1018100080000010000ff0200090c");

				QByteArray dateTime = QByteArray::fromHex(createDateTimeForDevice());

				QByteArray secondGeneral = QByteArray::fromHex("35fed400");

				QByteArray summForCRC = firstGeneral + dateTime + secondGeneral;

				sendMessage(createPacket(summForCRC)); // записываем время
			}

			if (counterForResend == 7)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea01c0002feffc9dc73f2e6e600c0018100080000010000ff020065d77e")));
			}

			if (counterForResend == 8)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7ea01c0002feffc9fe63f0e6e600c0018100080000010000ff020065d77e")));
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 9;
			}

			myTimer->start(20000);
			});
	}
	else
	{
		myTimer->stop();
		socket->close();

		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishMIR();
	}
}



quint16 TcpClientMIR::calculateCRC16(const QByteArray& data)
{
	quint16 crc = 0xFFFF;

	for (int i = 0; i < data.size(); ++i) {
		crc ^= static_cast<quint8>(data[i]);

		for (int j = 0; j < 8; ++j) {
			if (crc & 0x0001) {
				crc = (crc >> 1) ^ 0x8408;
			}
			else {
				crc >>= 1;
			}
		}
	}

	return crc ^ 0xFFFF;
}



QByteArray TcpClientMIR::createPacket(const QByteArray& data)
{
	QByteArray packet;
	packet.append(static_cast<char>(0x7E));  // HDLC флаг начала
	packet.append(data);

	quint16 crc = calculateCRC16(data);
	packet.append(static_cast<char>(crc & 0xFF));        // CRC младший байт
	packet.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC старший байт
	packet.append(static_cast<char>(0x7E));  // HDLC флаг конца

	return packet;
}


QByteArray TcpClientMIR::createDateTimeForDevice()
{
	QString dateForProtocol = QDate::currentDate().toString("yyyy-MM-dd");
	QString timeForProtocol = QTime::currentTime().toString("HH-mm-ss");

	QString year = dateForProtocol.first(4);
	QString month = dateForProtocol.sliced(5, 2); // начиная с 5-ой позиции и заканчивая 5-ой + 2.
	QString day = dateForProtocol.last(2);

	year = QString("%1").arg(year.toInt(), 4, 16, QChar('0'));
	month = QString("%1").arg(month.toInt(), 2, 16, QChar('0'));
	day = QString("%1").arg(day.toInt(), 2, 16, QChar('0'));

	QString hour = timeForProtocol.first(2);
	QString minute = timeForProtocol.sliced(3, 2); // начиная с 5-ой позиции и заканчивая 5-ой + 2.
	QString second = timeForProtocol.last(2);

	hour = QString("%1").arg(hour.toInt(), 2, 16, QChar('0'));
	minute = QString("%1").arg(minute.toInt(), 2, 16, QChar('0'));
	second = QString("%1").arg(second.toInt(), 2, 16, QChar('0'));

	QString finalString = year + month + day + "01" + hour + minute + second;

	return finalString.toLatin1();
}