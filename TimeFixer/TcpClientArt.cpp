#include "TcpClientArt.h"

TcpClientArt::TcpClientArt(QObject* parent) : QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	connect(socket, &QTcpSocket::connected, this, &TcpClientArt::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClientArt::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClientArt::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientArt::onErrorOccurred);
}



TcpClientArt::~TcpClientArt()
{
	if (socket->isOpen()) {
		socket->close();
	}
}



void TcpClientArt::connectToSavedHost()
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



void TcpClientArt::startConnectToHost(QString any, QString port)
{
	m_ip = any;
	m_port = port;
	connectToSavedHost();
}



void TcpClientArt::sendMessage(const QByteArray& message)
{
	if (socket->state() == QTcpSocket::ConnectedState)
	{
		socket->write(message);
		QString temp = '(' + QString::number(counterForResend+1) + ") >> ";
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "TX (" + m_ip + ')' + temp << message.toHex();
	}
	else
	{
		qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Not connected to host.";
	}
}



void TcpClientArt::onConnected()
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Connected to host (" + QString::number(reConnectCounter) + "): " << QHostAddress(m_ip).toString();

	changeDateTime();
}



void TcpClientArt::onDisconnected()
{
	qDebug() << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Disconnected from host.\n";
}



void TcpClientArt::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX (" + m_ip + ") << " << data.toHex();
}



void TcpClientArt::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\n" << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Socket error:" << socketError << socket->errorString() << '\n';

	if (socket->errorString().contains("Connection timed out") || socket->errorString().contains("Connection refused") || (socket->errorString().contains("The remote host closed the connection") && counterForResend >= 2))
	{
		artCycleFinished = true;
		counterForResend = 0;
		reConnectCounter = 0;
		secondArtCommand = false;
		emit finishART();
	}
}



void TcpClientArt::stopConnectionWithHost()
{
	qDebug() << '\n' << QDateTime::currentDateTime().toString("dd.MM.yyyy - hh.mm.ss - ") << "Try disconnect from host " << QHostAddress(m_ip).toString() << "\n";

	socket->abort();
}



QByteArray TcpClientArt::modbusCRCforArtTime(QString temp)
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



QByteArray TcpClientArt::modbusCRCforArtDate(QString temp)
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



void TcpClientArt::changeDateTime()
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
			emit finishART();
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
			changeDateTime();
			});
	}
	else
	{
		qDebug() << '\n' << "TcpClientArt::changeDateTime() -> Socket not open. Try reconnect.";
		connectToSavedHost();
	}
}