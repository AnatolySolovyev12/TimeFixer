#include "TcpClientNZIF.h"

TcpClientNZIF::TcpClientNZIF(QString serial, QObject* parent) : m_serial(serial), QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientNZIF::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientNZIF::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientNZIF::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientNZIF::onErrorOccurred);

	myTimer = new QTimer();
	connect(myTimer, &QTimer::timeout, this, &TcpClientNZIF::exchangeFromTimer);
}



TcpClientNZIF::~TcpClientNZIF()
{
	if (socket->isOpen())
		socket->close();
}



void TcpClientNZIF::connectToSavedHost()
{
	if (reConnectCounter >= 3)
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishNZIF();
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



void TcpClientNZIF::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClientNZIF::sendMessage(const QByteArray& message)
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



void TcpClientNZIF::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeNZIF();
}



void TcpClientNZIF::onDisconnected()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientNZIF::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX (" + m_ip + ") << " << data.toHex();

	if (read)
	{
		myTimer->stop();
		counterForResend++;
		reTransmitQuery = 0;
		read = false;
		write = true;

		changeTimeNZIF();
	}
}



void TcpClientNZIF::exchangeFromTimer()
{
	++reTransmitQuery;
	changeTimeNZIF();
}



void TcpClientNZIF::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	if (socket->errorString().contains("The remote host closed the connection"))
	{
		++reConnectCounter;
		connectToSavedHost();
		qDebug() << '\n' << "TcpClientNZIF::onErrorOccurred() -> Socket not open. Try reconnect.";
		return;
	}
	else
	{
		counterForResend = 0;
		reTransmitQuery = 0;
		reConnectCounter = 0;
		emit finishNZIF();
	}
}



void TcpClientNZIF::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	if (socket->state() == QAbstractSocket::ConnectedState)
		socket->abort();
}



void TcpClientNZIF::changeTimeNZIF()
{
	if (read)
		return;

	if (counterForResend != 4)
	{
		QTimer::singleShot(200, [this]() {

			bool ok = false;
			const int serial = m_serial.toInt(&ok, 10);
			QByteArray packet;

			if (counterForResend == 0) // запись
			{
				QByteArray firstGeneral = QByteArray::fromHex(QByteArray("030c"));

				QByteArray dateTime = QByteArray::fromHex(QString(QTime::currentTime().toString("ssmmHH") + "04" + QDate::currentDate().toString("ddMMyy") + "01").toLatin1());

				QByteArray summForCRC = firstGeneral + dateTime;

				packet.append(static_cast<char>(serial));
				packet += firstGeneral;
				packet += dateTime;

				sendMessage(createPacket(packet)); // записываем время
			}

			if (counterForResend == 1) // запись
			{
				QByteArray firstGeneral = QByteArray::fromHex(QByteArray("030c"));

				QByteArray dateTime = QByteArray::fromHex(QString(QTime::currentTime().toString("ssmmHH") + "04" + QDate::currentDate().toString("ddMMyy") + "01").toLatin1());

				QByteArray summForCRC = firstGeneral + dateTime;

				packet.append(static_cast<char>(serial));
				packet += firstGeneral;
				packet += dateTime;

				sendMessage(createPacket(packet)); // записываем время
			}

			if (counterForResend == 2) // чтение
			{
				QByteArray firstGeneral = QByteArray::fromHex(QByteArray("01303030303030"));

				QByteArray summForCRC = firstGeneral;

				packet.append(static_cast<char>(serial));
				packet += firstGeneral;

				sendMessage(createPacket(packet)); 
			}

			if (counterForResend == 3) // запись
			{
				QByteArray firstGeneral = QByteArray::fromHex(QByteArray("0400"));

				QByteArray summForCRC = firstGeneral;

				packet.append(static_cast<char>(serial));
				packet += firstGeneral;

				sendMessage(createPacket(packet)); // записываем время
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 4;
			}

			read = true;
			write = false;

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
		emit finishNZIF();
	}
}



quint16 TcpClientNZIF::calculateCRC16(const QByteArray& data)
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



QByteArray TcpClientNZIF::createPacket(const QByteArray& data)
{
	QByteArray packet;
	packet.append(data);

	quint16 crc = calculateCRC16(data);
	packet.append(static_cast<char>(crc & 0xFF));        // CRC младший байт
	packet.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC старший байт

	return packet;
}