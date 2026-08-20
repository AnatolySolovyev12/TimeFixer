#include "TcpClient.h"

TcpClient::TcpClient(QString any, QObject* parent) : serialStringForProtocol(any), QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
}

TcpClient::~TcpClient()
{
	if (socket->isOpen()) {
		socket->close();
	}
}



void TcpClient::connectToSavedHost()
{
	if (reConnectCounter >= 3)
	{
		counterForResend = 0;
		reConnectCounter = 0;
		secondArtCommand = false;
		stopConnectionWithHost();
	}
	else
	{
		if (socket->state() != QAbstractSocket::ConnectedState)
		{
			artCycleFinished = false;

			reConnectCounter++;

			if (socket->state() == QAbstractSocket::UnconnectedState) {
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}
			else {
				socket->abort();
				socket->connectToHost(QHostAddress(m_ip), m_port.toInt());
			}

			qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try connect to " + QHostAddress(m_ip).toString() << ':' << m_port;
		}
		else
			changeTimeArt();
	}
}



void TcpClient::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClient::sendMessage(const QByteArray& message)
{
	if (socket->state() == QTcpSocket::ConnectedState)
	{
		socket->write(message);
		QString temp = '(' + QString::number(counterForResend) + ") >> ";
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "TX " + temp << message.toHex();
	}
	else
	{
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Not connected to host.";
	}
}



void TcpClient::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeArt();
}



void TcpClient::onDisconnected()
{
	qDebug() << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClient::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX << " << data.toHex();
}



void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString();

	if (socket->errorString().contains("Connection timed out") || socket->errorString().contains("Connection refused") || socket->errorString().contains("The remote host closed the connection"))
	{
		artCycleFinished = true;
		counterForResend = 0;
		reConnectCounter = 0;
		secondArtCommand = false;
		emit finish();
	}
}



void TcpClient::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	socket->abort();
}



QByteArray TcpClient::modbusCRCforArtTime(QString temp)
{
	QByteArray command = "@00000C" + temp.toLatin1();

	quint32 sum = 0;

	for (char byte : command) {
		sum += static_cast<unsigned char>(byte);
	}

	quint8 checksum = static_cast<quint8>(sum & 0xFF);

	QByteArray checksumAscii =
		QString("%1")
		.arg(checksum, 2, 16, QChar('0'))
		.toUpper()
		.toLatin1();

	QByteArray frame = command + checksumAscii + '\r';

	//qDebug() << frame;
	//qDebug() << frame.toHex(' ');

	return frame;
}



QByteArray TcpClient::modbusCRCforArtDate(QString temp)
{
	QByteArray command = "@00000D1" + temp.toLatin1();

	quint32 sum = 0;

	for (char byte : command)
		sum += static_cast<unsigned char>(byte);

	quint8 checksum = static_cast<quint8>(sum & 0xFF);

	QByteArray checksumAscii =
		QString("%1")
		.arg(checksum, 2, 16, QChar('0'))
		.toUpper()
		.toLatin1();

	QByteArray frame = command + checksumAscii + '\r';

	return frame;
}



void TcpClient::changeTimeArt()
{
	if (artCycleFinished)
		return;

	if (counterForResend >= 6)
	{
		artCycleFinished = true;
		counterForResend = 0;
		reConnectCounter = 0;
		secondArtCommand = false;
		stopConnectionWithHost();

		QTimer::singleShot(800, [this]() {
			emit finish();
			});

		return;
	}

	if (socket->state() == QAbstractSocket::ConnectedState)
	{
		if (secondArtCommand)
			sendMessage(QByteArray(modbusCRCforArtTime(QTime::currentTime().toString("HHmmss"))));
		else
			sendMessage(QByteArray(modbusCRCforArtDate(QDate::currentDate().toString("ddMMyy"))));

		secondArtCommand = !secondArtCommand;

		++counterForResend;

		QTimer::singleShot(5000, [this]() {
			changeTimeArt();
			});
	}
	else
	{
		qDebug() << "TcpClient::changeTimeArt() -> Socket not open. Try reconnect.";
		connectToSavedHost();
	}
}



void TcpClient::changeTimeM2M()
{
	/*
	 -900	-58 966 807	    FC7C3CE9
	 -400	-26 153 367	    FE70EE69
	 -100	-6 528 568	    FF9C61C8
	 -20	-1 284 270	    FFEC6752
	 +20	+1 340 471	    00147437
	 +100	+6 593 849	    00649D39
	 +400	+26 273 979	    0190E8BB
	 +900	+59 289 788	    0388B0BC
	 */

	if (counterForResend != 7)
	{
		QTimer::singleShot(500, [this]() {

			if (counterForResend == 0)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA02102214193A585818014050207EE060207EE0704000000070804000000074EE97E"))); // коррект
			}

			if (counterForResend == 1)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA0450221411095BFE6E6006036A1090607608574050801018A0207808B0760857405080201AC0A80083030303030303030BE10040E01000000065F1F0400621E5DFFFF114C7E"))); //коррект
			}

			if (counterForResend == 2)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141321BA2E6E600C001C100010000000201FF02004F267E"))); // коррект
			}

			if (counterForResend == 3)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FC7C3CE97E")));//-900 - FC7C3CE9
			}


			if (counterForResend == 4)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF06011000649D397E")));//+20 - 00147437
			}

			if (counterForResend == 5)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF06011000649D397E")));//-100 - 00649D39
			}

			if (counterForResend == 6)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA008022141535C727E"))); // завершение при коррект
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 7;
			}

			myTimer->start(20000);
			});
	}
	else
	{
		myTimer->stop();
		socket->close();
		reTransmitQuery = 0;
	}
}
