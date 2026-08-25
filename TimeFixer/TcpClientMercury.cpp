#include "TcpClientMercury.h"

TcpClientMercury::TcpClientMercury(QString serial, QObject* parent) : m_serial(serial), QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientMercury::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientMercury::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientMercury::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientMercury::onErrorOccurred);

	myTimer = new QTimer();
	connect(myTimer, &QTimer::timeout, this, &TcpClientMercury::exchangeFromTimer);
}



TcpClientMercury::~TcpClientMercury()
{
	if (socket->isOpen())
		socket->close();
}



void TcpClientMercury::connectToSavedHost()
{
	if (reConnectCounter >= 3)
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishMercury();
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



void TcpClientMercury::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClientMercury::sendMessage(const QByteArray& message)
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



void TcpClientMercury::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeMercury();
}



void TcpClientMercury::onDisconnected()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientMercury::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX (" + m_ip + ") << " << data.toHex();

	myTimer->stop();
	counterForResend++;
	reTransmitQuery = 0;

	changeTimeMercury();
}



void TcpClientMercury::exchangeFromTimer()
{
	++reTransmitQuery;
	changeTimeMercury();
}



void TcpClientMercury::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	if (socket->errorString().contains("The remote host closed the connection"))
	{
		++reConnectCounter;
		connectToSavedHost();
		qDebug() << '\n' << "TcpClientMercury::onErrorOccurred() -> Socket not open. Try reconnect.";
		return;
	}
	else
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishMercury();
	}
}



void TcpClientMercury::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	if (socket->state() == QAbstractSocket::ConnectedState)
		socket->abort();
}



void TcpClientMercury::changeTimeMercury()
{
	if (counterForResend != 4)
	{
		QTimer::singleShot(50, [this]() {

			bool ok = false;
			const int serial = m_serial.toInt(&ok, 10);
			QByteArray packet;

			if (counterForResend == 0)
			{
				packet.append(static_cast<char>(serial));
				packet += QByteArray::fromHex("0102323232323232");

				sendMessage(createPacket(packet));
			}

			if (counterForResend == 1)
			{
				packet.append(static_cast<char>(serial));
				packet += QByteArray::fromHex("0803");

				sendMessage(createPacket(packet));
			}

			if (counterForResend == 2)
			{
				packet.append(static_cast<char>(serial));
				packet += QByteArray::fromHex("0812");

				sendMessage(createPacket(packet));
			}

			if (counterForResend == 3)
			{
				QByteArray firstGeneral = QByteArray::fromHex(QByteArray("030c"));

				QByteArray dateTime = QByteArray::fromHex(QString(QTime::currentTime().toString("ssmmHH") + "02" + QDate::currentDate().toString("ddMMyy") + "01").toLatin1());

				QByteArray summForCRC = firstGeneral + dateTime;

				packet.append(static_cast<char>(serial));
				packet += firstGeneral;
				packet += dateTime;

				sendMessage(createPacket(packet)); // записываем время
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 4;
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
		emit finishMercury();
	}
}



quint16 TcpClientMercury::calculateCRC16(const QByteArray& data)
{
	quint16 crc = 0xFFFF;

	for (const char byte : data) {
		crc ^= static_cast<quint8>(byte);

		for (int i = 0; i < 8; ++i) {
			if (crc & 0x0001) {
				crc = static_cast<quint16>((crc >> 1) ^ 0xA001);
			}
			else {
				crc = static_cast<quint16>(crc >> 1);
			}
		}
	}

	return crc;
}



QByteArray TcpClientMercury::createPacket(const QByteArray& data)
{
	QByteArray packet;
	packet.append(data);

	quint16 crc = calculateCRC16(data);
	packet.append(static_cast<char>(crc & 0xFF));        // CRC младший байт
	packet.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC старший байт

	return packet;
}