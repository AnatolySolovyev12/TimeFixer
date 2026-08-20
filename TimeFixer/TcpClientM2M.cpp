#include "TcpClientM2M.h"

TcpClientM2M::TcpClientM2M(QObject* parent) : QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientM2M::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientM2M::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientM2M::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientM2M::onErrorOccurred);
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
		secondArtCommand = false;
		stopConnectionWithHost();
	}
	else
	{
		if (socket->state() != QAbstractSocket::ConnectedState)
		{
			qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try connect to (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString() << ':' << m_port;

			artCycleFinished = false;

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
		QString temp = '(' + QString::number(counterForResend + 1) + ") >> ";
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "TX " + temp << message.toHex();
	}
	else
	{
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Not connected to host.";
	}
}



void TcpClientM2M::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeTimeM2M();
}



void TcpClientM2M::onDisconnected()
{
	qDebug() << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientM2M::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX << " << data.toHex();
}



void TcpClientM2M::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	if (socket->errorString().contains("Connection timed out") || socket->errorString().contains("Connection refused") || (socket->errorString().contains("The remote host closed the connection") && counterForResend >= 2))
	{
		artCycleFinished = true;
		counterForResend = 0;
		reConnectCounter = 0;
		secondArtCommand = false;
		emit finish();
	}
}



void TcpClientM2M::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	socket->abort();
}


void TcpClientM2M::changeDateTime()
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
			changeTimeM2M();
			});
	}
	else
	{
		qDebug() << '\n' << "TcpClientM2M::changeDateTime() -> Socket not open. Try reconnect.";
		connectToSavedHost();
	}
}



void TcpClientM2M::changeTimeM2M()
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
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FC7C3CE97E")));//-900 - FC7C3CE9
			}

			if (counterForResend == 4)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FC7C3CE97E")));
			}

			if (counterForResend == 5)
			{
				sendMessage(QByteArray::fromHex(QByteArray("7EA01D02214176E796E6E600C301C100080000010000FF060110FC7C3CE97E")));
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
