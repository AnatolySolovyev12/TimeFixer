#include "TcpClient.h"

TcpClient::TcpClient(QString any, QObject * parent) : serialStringForProtocol(any), QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	myTimer = new QTimer();
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

void TcpClient::connectToServer(const QString& host, quint16 port)
{
	socket->connectToHost(QHostAddress(host), port);
	qDebug() << "Connect to " + QHostAddress(host).toString() << ':' << port;
}

void TcpClient::sendMessage(const QByteArray& message)
{
	if (socket->state() == QTcpSocket::ConnectedState) {

		socket->write(message);
		qDebug() << "TX >> " << message.toHex();
	}
	else {
		qDebug() << "\nNot connected to server.";
	}
}

void TcpClient::onConnected()
{
	// connectedState = true;
	qDebug() << "\nConnected to server\n";

	changeTimeArt();

	/*
	if (serialStringForProtocol.contains('%'))
	{
		answerString = "Connected is done";
		socket->close();
		emit messageReceived(getKey());
		return;
	}

	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
	{
		vecExchange();
	}
	else if (serialStringForProtocol == "]101" || serialStringForProtocol == "]102" || serialStringForProtocol == "]103" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109")
	{
		getDaily();
	}
	else
	{
		exchange();
	}
	*/
}

void TcpClient::onDisconnected()
{
	connectedState = false;
	qDebug() << "\nDisconnected from server.\n";
}

void TcpClient::onReadyRead()
{
	QByteArray data = socket->readAll();

	qDebug() << "RX << " << data.toHex();

	myTimer->stop();
	counterForResend++;
	reTransmitQuery = 0;
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\nSocket error:" << socketError << socket->errorString();
	answerString += socket->errorString() + '.' + " No connection or bad signal";
	emit messageError();
	emit messageReceived(getKey());
}

QString TcpClient::returnResultString()
{
	return answerString;
}

void TcpClient::setResultString(QString any)
{
	answerString += any + "\n\n";
}

void TcpClient::startToConnect(QString any, QString port)
{
	ip = any;
	connectToServer(ip, port.toInt());
}

void TcpClient::resetAnswerString()
{
	answerString = "";
}



QByteArray TcpClient::modbusCRCforArt()
{
	QString time = QTime::currentTime().toString("HHmmss");

	QByteArray command = "@00000C" + time.toLatin1();

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

	qDebug() << frame;
	qDebug() << frame.toHex(' ');

	return frame;
}



void TcpClient::changeTimeArt()
{
	if (counterForResend != 1)
	{
		QTimer::singleShot(500, [this]() {

			if (counterForResend == 0) // daily
			{
				sendMessage(QByteArray(modbusCRCforArt()));
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 1;
				answerString += "No or stopped responses from remote socket.";
			}

			myTimer->start(20000);
			});
	}
	else
	{
		myTimer->stop();
		socket->close();
		qDebug() << '\n' << answerString;
		ip = "";
		reTransmitQuery = 0;

		emit messageReceived(getKey());
	}
}



void TcpClient::setKey(int64_t any)
{
	key = any;
}



const int64_t TcpClient::getKey()
{
	return key;
}



QString TcpClient::getSerialStringForProtocol()
{
	return serialStringForProtocol;
}



QString TcpClient::hexDateFunc(QString date) // формарования целевой даты в виде HEX последовательности
{
	bool ok;
	int purposeDay = date.toInt();

	QString dateCurr = QDate::currentDate().toString("yyyy.MM.dd");

	QString dayString = dateCurr.sliced(8);
	int currDay = dayString.toInt();

	QString monthString = dateCurr.sliced(5);
	monthString.chop(3);
	int month = monthString.toInt();

	dateCurr.chop(6);
	int year = dateCurr.toInt();

	QString hexString = QString("%1").arg(year, 4, 16, QChar('0')).toUpper(); // значение, количество знаков, преобразование, заполнитель начальный

	if (purposeDay > currDay) --month;

	hexString += QString("%1").arg(month, 2, 16, QChar('0')).toUpper();
	hexString += QString("%1").arg(purposeDay, 2, 16, QChar('0')).toUpper();

	return hexString;
}



quint16 TcpClient::crc16Kermit(const QByteArray& data) // расчёт контрольной суммы CRC-16 при запросе суточных показаний
{
	const uint16_t poly = 0x1021;
	const uint16_t init = 0xFFFF;
	const bool ref_in = true;
	const bool ref_out = true;
	const uint16_t xor_out = 0xFFFF;

	uint16_t crc = init;

	for (char ch : data)
	{
		uint8_t byte = static_cast<uint8_t>(ch);
		// Отражение входного байта (ref_in)
		if (ref_in)
		{
			uint8_t reflected = 0;

			for (int b = 0; b < 8; ++b)
			{
				if (byte & (1 << b))
					reflected |= (1 << (7 - b));
			}
			byte = reflected;
		}

		crc ^= (byte << 8);

		for (int b = 0; b < 8; ++b)
		{
			if (crc & 0x8000)
				crc = (crc << 1) ^ poly;
			else
				crc <<= 1;
			crc &= 0xFFFF;
		}
	}
	// Отражение результата (ref_out)
	if (ref_out)
	{
		uint16_t reflected = 0;

		for (int b = 0; b < 16; ++b)
		{
			if (crc & (1 << b))
				reflected |= (1 << (15 - b));
		}
		crc = reflected;
	}
	// Инверсия (xor_out)
	crc ^= xor_out;
	return crc;
}