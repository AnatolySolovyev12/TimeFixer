﻿#include "TcpClient.h"

TcpClient::TcpClient(QString any, QObject * parent) : serialStringForProtocol(any), QObject(parent), socket(new QTcpSocket(this))
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	myTimer = new QTimer();
	connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
	connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
	connect(myTimer, &QTimer::timeout, this, &TcpClient::exchangeFromTimer);
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

	if (data.toHex().length() < 35 && (serialStringForProtocol == ">101" || serialStringForProtocol == ">103" || serialStringForProtocol == ">102" || serialStringForProtocol == ">104" || serialStringForProtocol == ">109" || serialStringForProtocol == "_101" || serialStringForProtocol == "_103" || serialStringForProtocol == "_102" || serialStringForProtocol == "_104" || serialStringForProtocol == "_109")) ////////////////////////надо корректировать защиту
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		exchange();
		return;
	}

	if (data.toHex().length() < 42 && (serialStringForProtocol == "101" || serialStringForProtocol == "103" || serialStringForProtocol == "109") && (counterForResend != 16))
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		exchange();
		return;
	}

	if (data.toHex().length() < 42 && (serialStringForProtocol == "102" || serialStringForProtocol == "104" || serialStringForProtocol == "106") && (counterForResend != 30))
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		exchange();
		return;
	}

	if (data.toHex().length() > 68 && (serialStringForProtocol == "101" || serialStringForProtocol == "103" || serialStringForProtocol == "102" || serialStringForProtocol == "104" || serialStringForProtocol == "106" || serialStringForProtocol == "109") && counterForResend >= 2)
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		exchange();
		return;
	}

	if (data.toHex().length() < 35 && (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109"))
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		vecExchange();
		return;
	}

	if (data.toHex().length() < 35 && (serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]102" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109") && counterForResend >= 2)
	{
		qDebug() << "\nincorrect RX. Resend";
		reTransmitQuery++;
		myTimer->stop();
		getDaily();
		return;
	}

	if (serialStringForProtocol == "101" || serialStringForProtocol == "103" || serialStringForProtocol == "109")
	{
		if (counterForResend >= 2 && counterForResend != 16)
		{
			QString temporaryAnswer = data.toHex();
			summAnswer(temporaryAnswer);
		}
	}

	if (serialStringForProtocol == "102" || serialStringForProtocol == "104" || serialStringForProtocol == "106")
	{
		if (counterForResend >= 2 && counterForResend != 30)
		{
			QString temporaryAnswer = data.toHex();
			summAnswer(temporaryAnswer);
		}

	}

	if (serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]102" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109")
	{
		if (dailyArchiveString != "00")
		{
			if (counterForResend == 2)
			{
				QString temporaryAnswer = data.toHex();
				summAnswer(temporaryAnswer);
			}
		}
		else
		{
			if (counterForResend == 2 || counterForResend == 3 || counterForResend == 4)
			{
				QString temporaryAnswer = data.toHex();
				summAnswer(temporaryAnswer);
			}
		}

	}

	if ((serialStringForProtocol == "_101" || serialStringForProtocol == "_103" || serialStringForProtocol == "_102" || serialStringForProtocol == "_104" || serialStringForProtocol == "_109") && counterForResend == 4)
	{
		answerString += "Relay was connect";
	}

	if ((serialStringForProtocol == ">101" || serialStringForProtocol == ">103" || serialStringForProtocol == ">102" || serialStringForProtocol == ">104" || serialStringForProtocol == ">109") && counterForResend == 4)
	{
		answerString += "Relay was disconnect";
	}

	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
	{
		if (counterForResend >= 2 && counterForResend != 13)
		{
			QString temporaryAnswer = data.toHex();
			summAnswervector(temporaryAnswer);
		}
	}

	myTimer->stop();
	counterForResend++;
	reTransmitQuery = 0;

	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
		vecExchange();
	else if ((serialStringForProtocol == "]101" || serialStringForProtocol == "]102" || serialStringForProtocol == "]103" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109") && dailyArchiveString != "00")
		getDaily();
	else if ((serialStringForProtocol == "]101" || serialStringForProtocol == "]102" || serialStringForProtocol == "]103" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109") && dailyArchiveString == "00")
		getCurr();
	else
		exchange();
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	qDebug() << "\nSocket error:" << socketError << socket->errorString();
	answerString += socket->errorString() + '.' + " No connection or bad signal";
	emit messageError();
	emit messageReceived(getKey());
}

void TcpClient::summAnswer(QString& any)
{
	if (serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]102" || serialStringForProtocol == "]104" || serialStringForProtocol == "]106" || serialStringForProtocol == "]109")
	{
		if (dailyArchiveString != "00")
		{
			QString dayVal;
			QString nigntVal;
			QString sumVal;

			if (any.length() > 480)
			{
				dayVal = any.sliced(74);
				dayVal.chop(416);

				nigntVal = any.sliced(92);
				nigntVal.chop(398);

				sumVal = any.sliced(146);
				sumVal.chop(344);
			}
			else
			{
				dayVal = any.sliced(74);
				dayVal.chop(348);

				nigntVal = any.sliced(92);
				nigntVal.chop(330);

				sumVal = any.sliced(146);
				sumVal.chop(276);
			}

			qDebug() << "after sliced and chop:   " + sumVal << "   " << dayVal << "   " << nigntVal << '\n';

			int counter = 1;

			for (auto& val : { sumVal, dayVal, nigntVal })
			{
				bool ok;
				bool minus = false;
				QString temporaryAnswer = val;

				if ((val.toUInt(&ok, 16) > 4200000000) && counterForResend > 5)
				{
					temporaryAnswer = QString("%1")
						.arg(4294967295 - temporaryAnswer.toUInt(&ok, 16));
					minus = true;
				}
				else
				{
					temporaryAnswer = QString("%1")
						.arg(temporaryAnswer.toULongLong(&ok, 16));
				}

				QString tariffText = "";
				if (counter == 1) tariffText = "SUM - ";
				else if (counter == 2) tariffText = "T1 - ";
				else  tariffText = "T2 - ";

				++counter;
				//Отделяем целую часть от дробной исходя из типа счётчика
				serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]109" ? temporaryAnswer.insert((temporaryAnswer.length() - 6), ',') : temporaryAnswer.insert((temporaryAnswer.length() - 7), ',');
				answerString += tariffText + temporaryAnswer + '\n';
			}

			qDebug() << "after convert " + answerString << '\n';
		}
		else
		{
			QString tempVal;

			tempVal = any.sliced(38);
			tempVal.chop(6);

			qDebug() << "after sliced and chop:   " + tempVal << '\n';

			bool ok;
			bool minus = false;
			QString temporaryAnswer = tempVal;

			if ((tempVal.toUInt(&ok, 16) > 4200000000) && counterForResend > 5)
			{
				temporaryAnswer = QString("%1")
					.arg(4294967295 - temporaryAnswer.toUInt(&ok, 16));
				minus = true;
			}
			else
			{
				temporaryAnswer = QString("%1")
					.arg(temporaryAnswer.toULongLong(&ok, 16));
			}

			if (counterForResend < 5)
			{

				switch (counterForResend)
				{
				case 2:
				{
					answerString += "SUM - ";
					break;
				}
				case 3:
				{
					answerString += "\nT1 - ";
					break;
				}
				case 4:
				{
					answerString += "\nT2 - ";
					break;
				}
				break;
				};

				serialStringForProtocol == "]101" || serialStringForProtocol == "]103" || serialStringForProtocol == "]109" ? temporaryAnswer.insert((temporaryAnswer.length() - 6), ',') : temporaryAnswer.insert((temporaryAnswer.length() - 7), ',');

				answerString += temporaryAnswer + ' ';
				qDebug() << "after convert " + temporaryAnswer << '\n';
			}
		}
	}

	if (serialStringForProtocol == "101" || serialStringForProtocol == "103" || serialStringForProtocol == "109")
	{
		bool ok;
		bool minus = false;
		QString temporaryAnswer = any.sliced(34);
		temporaryAnswer.chop(6);
		QString frankenshteinString;

		qDebug() << "after sliced and chop = " + temporaryAnswer << '\n';

		if ((temporaryAnswer.toUInt(&ok, 16) > 4200000000) && counterForResend > 5)
		{
			temporaryAnswer = QString("%1")
				.arg(4294967295 - temporaryAnswer.toUInt(&ok, 16));
			minus = true;
		}
		else
		{
			temporaryAnswer = QString("%1")
				.arg(temporaryAnswer.toULongLong(&ok, 16));
		}

		if (counterForResend <= 5)
		{
			if (temporaryAnswer.length() == 6)
			{
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 5)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 4)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			frankenshteinString = temporaryAnswer.last(6);
			temporaryAnswer.chop(6);

			if (temporaryAnswer == "")
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front("0");
			}
			else {
				frankenshteinString.push_front(",");
				frankenshteinString.push_front(temporaryAnswer);
			}

			switch (counterForResend)
			{
			case 2:
			{
				answerString += "A+ ";
				break;
			}
			case 3:
			{
				answerString += "\nA- ";
				break;
			}
			case 4:
			{
				answerString += "\nR+ ";
				break;
			}
			case 5:
			{
				answerString += "\nR- ";
				break;
			}
			break;
			};

			answerString += frankenshteinString + ' ';
			qDebug() << "after convert " + frankenshteinString << '\n';
		}

		if (counterForResend == 6 || counterForResend == 11 || counterForResend == 12 || counterForResend == 13)
		{
			switch (counterForResend)
			{
			case 6:
			{
				answerString += "\nRelay ";
				break;
			}
			case 11:
			{
				answerString += "\nS (full) ";
				break;
			}
			case 12:
			{
				answerString += "\nP (active) ";
				break;
			}
			case 13:
			{
				answerString += "\nQ (reactive) ";
				break;
			}
			break;
			};

			if (minus)
			{
				answerString += '-' + temporaryAnswer + ' ';
			}
			else
			{
				answerString += temporaryAnswer + ' ';
			}

			qDebug() << "after convert " + temporaryAnswer << '\n';
		}

		if (counterForResend == 7 || counterForResend == 8 || counterForResend == 9 || counterForResend == 10 || counterForResend == 14 || counterForResend == 15)
		{

			if (temporaryAnswer.length() == 2)
			{
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 1)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			frankenshteinString = temporaryAnswer.last(3);

			temporaryAnswer.chop(3);

			if (temporaryAnswer == "")
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front("0");
			}
			else {
				frankenshteinString.push_front(",");
				frankenshteinString.push_front(temporaryAnswer);
			}

			switch (counterForResend)
			{
			case 7:
			{
				answerString += "\nI ";
				break;
			}
			case 8:
			{
				answerString += "\nI neutral ";
				break;
			}
			case 9:
			{
				answerString += "\nU ";
				break;
			}
			case 10:
			{
				answerString += "\nFr ";
				break;
			}
			case 14:
			{
				answerString += "\ncos (f) ";
				break;
			}
			case 15:
			{
				answerString += "\nI diff ";
				break;
			}
			break;
			}

			if (minus)
			{
				answerString += '-' + frankenshteinString + ' ';
			}
			else
			{
				answerString += frankenshteinString + ' ';
			}
			qDebug() << "after convert " + frankenshteinString << '\n';
		}
	}

	if (serialStringForProtocol == "102" || serialStringForProtocol == "104" || serialStringForProtocol == "106")
	{
		bool ok;
		bool minus = false;
		QString temporaryAnswer = any.sliced(34);
		temporaryAnswer.chop(6);
		QString frankenshteinString;

		qDebug() << "after sliced and chop = " + temporaryAnswer << '\n';

		if ((temporaryAnswer.toUInt(&ok, 16) > 4200000000) && counterForResend > 17)
		{
			temporaryAnswer = QString("%1")
				.arg(4294967295 - temporaryAnswer.toUInt(&ok, 16));
			minus = true;
		}
		else
		{
			temporaryAnswer = QString("%1")
				.arg(temporaryAnswer.toULongLong(&ok, 16));
		}

		if (counterForResend <= 17)
		{
			if (temporaryAnswer.length() == 6)
			{
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 5)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 4)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			frankenshteinString = temporaryAnswer.last(7);
			temporaryAnswer.chop(7);

			if (temporaryAnswer == "")
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front("0");
			}
			else
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front(temporaryAnswer);
			}

			switch (counterForResend)
			{
			case 2:
			{
				answerString += "A+ ";
				break;
			}
			case 3:
			{
				answerString += "\nA- ";
				break;
			}
			case 4:
			{
				answerString += "\nR+ ";
				break;
			}
			case 5:
			{
				answerString += "\nR- ";
				break;
			}
			case 6:
			{
				answerString += "\nA+ L1 ";
				break;
			}
			case 7:
			{
				answerString += "\nA+ L2 ";
				break;
			}
			case 8:
			{
				answerString += "\nA+ L3 ";
				break;
			}
			case 9:
			{
				answerString += "\nA- L1 ";
				break;
			}
			case 10:
			{
				answerString += "\nA- L2 ";
				break;
			}
			case 11:
			{
				answerString += "\nA- L3 ";
				break;
			}
			case 12:
			{
				answerString += "\nR+ L1 ";
				break;
			}
			case 13:
			{
				answerString += "\nR+ L2 ";
				break;
			}
			case 14:
			{
				answerString += "\nR+ L3 ";
				break;
			}
			case 15:
			{
				answerString += "\nR- L1 ";
				break;
			}
			case 16:
			{
				answerString += "\nR- L2 ";
				break;
			}
			case 17:
			{
				answerString += "\nR- L3 ";
				break;
			}

			break;
			};

			answerString += frankenshteinString + ' ';
			qDebug() << "after convert " + frankenshteinString << '\n';
		}

		if (counterForResend == 18 || counterForResend == 26 || counterForResend == 27 || counterForResend == 28)
		{
			switch (counterForResend)
			{
			case 18:
			{
				answerString += "\nRelay ";
				break;
			}
			case 26:
			{
				answerString += "\nS (full) ";
				break;
			}
			case 27:
			{
				answerString += "\nP (active) ";
				break;
			}
			case 28:
			{
				answerString += "\nQ (reactive) ";
				break;
			}
			break;
			};

			if (minus)
			{
				answerString += '-' + temporaryAnswer + ' ';
			}
			else
			{
				answerString += temporaryAnswer + ' ';
			}

			qDebug() << "after convert " + temporaryAnswer << '\n';
		}

		if (counterForResend == 19 || counterForResend == 20 || counterForResend == 21 || counterForResend == 22 || counterForResend == 23 || counterForResend == 24 || counterForResend == 25 || counterForResend == 29)
		{

			if (temporaryAnswer.length() == 2)
			{
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 1)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			frankenshteinString = temporaryAnswer.last(3);
			temporaryAnswer.chop(3);

			if (temporaryAnswer == "")
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front("0");
			}
			else
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front(temporaryAnswer);
			}

			switch (counterForResend)
			{
			case 19:
			{
				answerString += "\nI L1 ";
				break;
			}
			case 20:
			{
				answerString += "\nI L2 ";
				break;
			}
			case 21:
			{
				answerString += "\nI L3 ";
				break;
			}
			case 22:
			{
				answerString += "\nU L1 ";
				break;
			}
			case 23:
			{
				answerString += "\nU L2 ";
				break;
			}
			case 24:
			{
				answerString += "\nU L3 ";
				break;
			}
			case 25:
			{
				answerString += "\nFr ";
				break;
			}
			case 29:
			{
				answerString += "\ncos (f) ";
				break;
			}
			break;
			}

			if (minus)
			{
				answerString += '-' + frankenshteinString + ' ';
			}
			else
			{
				answerString += frankenshteinString + ' ';
			}
			qDebug() << "after convert " + frankenshteinString << '\n';
		}
	}
}

QString TcpClient::returnResultString()
{
	return answerString;
}

void TcpClient::setResultString(QString any)
{
	answerString += any + "\n\n";
}

void TcpClient::exchange()
{
	if (serialStringForProtocol == "101" || serialStringForProtocol == "103" || serialStringForProtocol == "109")
	{

		if (counterForResend != 17)
		{
			QTimer::singleShot(500, [this]() {

				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				if (counterForResend == 0) //start 1
				{
					//7E A0 1F 02 21 41 93 CC 30 81 80 12 05 01 F0 06 01 F0 07 04 00 00 00 01 08 04 00 00 00 01 F9 CB 7E

					QByteArray hexValue1 = "\x7e\xa0\x1f\x02\x21\x41\x93\xcc\x30\x81\x80\x12\x05\x01\xf0\x06\x01\xf0\x07\x04";
					QByteArray hexValue2 = "\x01\x08\x04";
					QByteArray hexValue3 = "\x01\xf9\xcb\x7e";

					QByteArray testArray = hexValue1 + nullVal + nullVal + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3;

					sendMessage(testArray);
				}

				if (counterForResend == 1) //start 2
				{
					//7E A0 45 02 21 41 10 95 BF E6 E6 00 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 30 30 30 30 30 30 30 30 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F FF FF 00 D1 7E
					QString hexValueZero = QString::number(0, 16);
					QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

					QByteArray hexValue1 = "\x7E\xA0\x45\x02\x21\x41\x10\x95\xBF\xE6\xE6";

					QByteArray hexValue2 = "\x60\x36\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x01\xAC\x0A\x80\x08\x30\x30\x30\x30\x30\x30\x30\x30\xBE\x10\x04\x0E\x01";
					QByteArray hexValue3 = "\x06\x5F\x1F\x04";
					QByteArray hexValue4 = "\x7E\x1F\xFF\xFF";
					QByteArray hexValue5 = "\xD1\x7E";

					//QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;
					//sendMessage(testArray);
					sendMessage(QByteArray::fromHex(QByteArray("7EA0450221411095BFE6E6006036A1090607608574050801018A0207808B0760857405080201AC0A80083030303030303030BE10040E01000000065F1F0400007E1FFFFF00D17E")));
				}

				if (counterForResend == 2) // A+ cur
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 03 01 00 01 08 00 FF 02 00 29 FA 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x01\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x29\xFA\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 3) // A- cur
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 02 08 00 FF 02 00 54 F6 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x02\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x54\xF6\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 4) // R+ cur
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 03 08 00 FF 02 00 7F F2 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x03\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x7F\xF2\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 5) // R- cur
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 04 08 00 FF 02 00 AE EE 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x04\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xAE\xEE\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					//qDebug() << "TA DA DA DA DA DA DA";
//sendMessage(QByteArray::fromHex(QByteArray("7EA04D022141949927E6E600C001C100070100620200FF0201010204020412000809060000010000FF0F02120000090C07EA051DFF000000FF000000090C07EA0604FF000000FF000000010062237E"))); // запрос на 29.05.2026 в ЛЭРС к М2М-1
					sendMessage(testArray);
				}

				if (counterForResend == 6) // Status relay
				{
					//7E A0 1A 02 21 41 BA 5B AA E6 E6 00 C0 01 41 00 46 00 00 60 03 0A FF 02 00 35 F7 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xBA\x5B\xAA\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x46";
					QByteArray hexValue4 = "\x60\x03\x0A\xFF\x02";
					QByteArray hexValue5 = "\x35\xF7\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 7) // I sum
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 41 00 03 01 00 0B 07 00 FF 02 00 DB B9 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0B\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xDB\xB9\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 8) // I neutral
				{
					//7E A0 1A 02 21 41 FE 7B AE E6 E6 00 C0 01 41 00 03 01 00 5B 07 00 FF 02 00 BA F9 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xFE\x7B\xAE\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x5B\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xBA\xF9\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 9) // U
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 41 00 03 01 00 0C 07 00 FF 02 00 0A A5 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0C\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x0A\xA5\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 10) // Frequency
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 03 01 00 0E 07 00 FF 02 00 5C AD 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0E\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x5C\xAD\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 11) // S
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 09 07 00 FF 02 00 8D B1 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x09\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x8D\xB1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 12) // P
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 01 07 00 FF 02 00 D5 90 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x01\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xD5\x90\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 13) // Q
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 03 07 00 FF 02 00 83 98 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x03\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x83\x98\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 14) // cos (f)
				{
					//7E A0 1A 02 21 41 BA 5B AA E6 E6 00 C0 01 41 00 03 01 00 0D 07 00 FF 02 00 21 A1 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xBA\x5B\xAA\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0D\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x21\xA1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 15) // I diff
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 41 00 03 01 00 5B 07 83 FF 02 00 19 F1 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x5B\x07\x83\xFF\x02";
					QByteArray hexValue5 = "\x19\xF1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 16) // end
				{
					//7E A0 08 02 21 41 53 5C 72 7E

					QByteArray hexValue1 = "\x7E\xA0\x08\x02\x21\x41\x53\x5C\x72\x7E";
					QByteArray testArray = hexValue1;

					sendMessage(testArray);
				}

				if (reTransmitQuery >= 4)
				{
					counterForResend = 17;
					answerString += "\nNo, stopped or incorrect responses from remote socket";
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

	if (serialStringForProtocol == "102" || serialStringForProtocol == "104" || serialStringForProtocol == "106")
	{
		if (counterForResend != 31)
		{
			QTimer::singleShot(500, [this]() {

				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				if (counterForResend == 0) // start 1
				{
					//7E A0 1F 02 21 41 93 CC 30 81 80 12 05 01 F0 06 01 F0 07 04 00 00 00 01 08 04 00 00 00 01 F9 CB 7E

					QByteArray hexValue1 = "\x7E\xA0\x1F\x02\x21\x41\x93\xCC\x30\x81\x80\x12\x05\x01\xF0\x06\x01\xF0\x07\x04";
					QByteArray hexValue2 = "\x01\x08\x04";
					QByteArray hexValue3 = "\x01\xF9\xCB\x7E";

					QByteArray testArray = hexValue1 + nullVal + nullVal + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3;

					sendMessage(testArray);
				}

				if (counterForResend == 1) // start 2
				{
					//7E A0 45 02 21 41 10 95 BF E6 E6 00 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 30 30 30 30 30 30 30 30 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F FF FF 00 D1 7E

					QString hexValueZero = QString::number(0, 16);
					QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

					QByteArray hexValue1 = "\x7E\xA0\x45\x02\x21\x41\x10\x95\xBF\xE6\xE6";

					QByteArray hexValue2 = "\x60\x36\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x01\xAC\x0A\x80\x08\x30\x30\x30\x30\x30\x30\x30\x30\xBE\x10\x04\x0E\x01";
					QByteArray hexValue3 = "\x06\x5F\x1F\x04";
					QByteArray hexValue4 = "\x7E\x1F\xFF\xFF";
					QByteArray hexValue5 = "\xD1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 2) // A+ cur
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 03 01 00 01 08 00 FF 02 00 29 FA 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x01\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x29\xFA\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 3) // A- cur
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 02 08 00 FF 02 00 54 F6 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x02\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x54\xF6\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 4) // R+ cur
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 03 08 00 FF 02 00 7F F2 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x03\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x7F\xF2\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 5) // R- cur
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 04 08 00 FF 02 00 AE EE 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x04\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xAE\xEE\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 6) //A + cur (phase 1)
				{
					//7E A0 1A 02 21 41 BA 5B AA E6 E6 00 C0 01 41 00 03 01 00 15 08 00 FF 02 00 35 A8 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xBA\x5B\xAA\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x15\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x35\xA8\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 7) //A + cur (phase 2)
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 41 00 03 01 00 29 08 00 FF 02 00 11 5E 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x29\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x11\x5E\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 8) //A + cur (phase 3)
				{
					//7E A0 1A 02 21 41 FE 7B AE E6 E6 00 C0 01 41 00 03 01 00 3D 08 00 FF 02 00 0D 0C 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xFE\x7B\xAE\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x3D\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x0D\x0C\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 9) //A - cur (phase 1)
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 41 00 03 01 00 16 08 00 FF 02 00 48 A4 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x16\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x48\xA4\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 10) //A - cur (phase 2)
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 03 01 00 2A 08 00 FF 02 00 6C 52 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x2A\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x6C\x52\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 11)  //A - cur (phase 3)
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 3E 08 00 FF 02 00 70 00 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x3E\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x70";
					QByteArray hexValue7 = "\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6 + nullVal + hexValue7;

					sendMessage(testArray);
				}

				if (counterForResend == 12)  //R + cur (phase 1)
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 17 08 00 FF 02 00 63 A0 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x17\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x63\xA0\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 13)  //R + cur (phase 2)
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 2B 08 00 FF 02 00 47 56 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x2B\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x47\x56\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 14)  //R + cur (phase 3)
				{
					//7E A0 1A 02 21 41 BA 5B AA E6 E6 00 C0 01 41 00 03 01 00 3F 08 00 FF 02 00 5B 04 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xBA\x5B\xAA\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x3F\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x5B\x04\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 15)  //R - cur (phase 1)
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 41 00 03 01 00 18 08 00 FF 02 00 EA 9D 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x18\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xEA\x9D\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}


				if (counterForResend == 16) //R - cur (phase 2)
				{
					//7E A0 1A 02 21 41 FE 7B AE E6 E6 00 C0 01 41 00 03 01 00 2C 08 00 FF 02 00 96 4A 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xFE\x7B\xAE\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x2C\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x96\x4A\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 17) //R - cur (phase 3)
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 41 00 03 01 00 40 08 00 FF 02 00 D3 FC 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x40\x08";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xD3\xFC\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 18) //relay status
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 46 00 00 60 03 0A FF 02 00 35 F7 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x46";
					QByteArray hexValue4 = "\x60\x03\x0A\xFF\x02";
					QByteArray hexValue5 = "\x35\xF7\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 19) // I (phase 1)
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 1F 07 00 FF 02 00 C7 EB 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x1F\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xC7\xEB\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 20) // I (phase 2)
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 33 07 00 FF 02 00 53 5F 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x33\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x53\x5F\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 21) // I (phase 3)
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 47 07 00 FF 02 00 FE 8A 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x47\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xFE\x8A\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 22) // U (phase 1)
				{
					//7E A0 1A 02 21 41 BA 5B AA E6 E6 00 C0 01 41 00 03 01 00 20 07 00 FF 02 00 9E 11 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xBA\x5B\xAA\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x20\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x9E\x11\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 23) // U (phase 2)
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 41 00 03 01 00 34 07 00 FF 02 00 82 43 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x34\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x82\x43\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 24) // U (phase 3)
				{
					//7E A0 1A 02 21 41 FE 7B AE E6 E6 00 C0 01 41 00 03 01 00 48 07 00 FF 02 00 77 B7 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xFE\x7B\xAE\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x48\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x77\xB7\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 25) //frequensy
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 41 00 03 01 00 0E 07 00 FF 02 00 5C AD 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0E\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x5C\xAD\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 26) // S
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 41 00 03 01 00 09 07 00 FF 02 00 8D B1 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x09\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x8D\xB1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 27) //P
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 41 00 03 01 00 01 07 00 FF 02 00 D5 90 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x01\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xD5\x90\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 28) //Q
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 03 07 00 FF 02 00 83 98 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x03\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x83\x98\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 29) // cos(f)
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 0D 07 00 FF 02 00 21 A1 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x0D\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x21\xA1\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 30) // завершающий
				{
					//7E A0 08 02 21 41 53 5C 72 7E

					QByteArray hexValue1 = "\x7E\xA0\x08\x02\x21\x41\x53\x5C\x72\x7E";
					QByteArray testArray = hexValue1;

					sendMessage(testArray);
				}

				if (reTransmitQuery >= 4)
				{
					counterForResend = 31;
					answerString += "\nNo or stopped responses from remote socket";
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

	if (serialStringForProtocol == "_101" || serialStringForProtocol == "_103" || serialStringForProtocol == "_102" || serialStringForProtocol == "_104" || serialStringForProtocol == "_109" || serialStringForProtocol == ">101" || serialStringForProtocol == ">103" || serialStringForProtocol == ">102" || serialStringForProtocol == ">104" || serialStringForProtocol == ">109") // включение/отключение реле
	{

		if (counterForResend != 5)
		{
			QTimer::singleShot(500, [this]() {

				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				if (counterForResend == 0)
				{
					//7E A0 1F 02 21 61 93 FF 13 81 80 12 05 01 F0 06 01 F0 07 04 00 00 00 01 08 04 00 00 00 01 F9 CB 7E

					QByteArray hexValue1 = "\x7E\xA0\x1F\x02\x21\x61\x93\xFF\x13\x81\x80\x12\x05\x01\xF0\x06\x01\xF0\x07\x04";
					QByteArray hexValue2 = "\x01\x08\x04";
					QByteArray hexValue3 = "\x01\xF9\xCB\x7E";

					QByteArray testArray = hexValue1 + nullVal + nullVal + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3;

					sendMessage(testArray);
				}

				if (counterForResend == 1)
				{
					//7E A0 4D 02 21 61 10 86 C6 E6 E6 00 60 3E A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 02 AC 12 80 10 5A 82 E6 42 88 CD BB 0F A4 96 2C DE F3 5F 3C 50 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F FF FF A3 DF 7E

					QByteArray hexValue1 = "\x7E\xA0\x4D\x02\x21\x61\x10\x86\xC6\xE6\xE6";
					QByteArray hexValue2 = "\x60\x3E\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x02\xAC\x12\x80\x10\x5A\x82\xE6\x42\x88\xCD\xBB\x0F\xA4\x96\x2C\xDE\xF3\x5F\x3C\x50\xBE\x10\x04\x0E\x01";
					QByteArray hexValue3 = "\x06\x5F\x1F\x04";
					QByteArray hexValue4 = "\x7E\x1F\xFF\xFF\xA3\xDF\x7E";


					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + nullVal + hexValue4;

					sendMessage(testArray);
				}


				if (counterForResend == 2 && (serialStringForProtocol == "_101" || serialStringForProtocol == "_103" || serialStringForProtocol == "_102" || serialStringForProtocol == "_104" || serialStringForProtocol == "_109")) //  проверка подлинности при включении реле
				{
					//7E A0 2C 02 21 61 32 61 6E E6 E6 00 C3 01 C1 00 0F 00 00 28 00 00 FF 01 01 09 10 B3 55 E5 CE 9A 8F C9 5A 4B BD 68 37 E6 C1 0E 1C 2E 13 7E

					QByteArray hexValue1 = "\x7E\xA0\x2C\x02\x21\x61\x32\x61\x6E\xE6\xE6";
					QByteArray hexValue2 = "\xC3\x01\xC1";
					QByteArray hexValue3 = "\x0F";
					QByteArray hexValue4 = "\x28";
					QByteArray hexValue5 = "\xFF\x01\x01\x09\x10\xB3\x55\xE5\xCE\x9A\x8F\xC9\x5A\x4B\xBD\x68\x37\xE6\xC1\x0E\x1C\x2E\x13\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 2 && (serialStringForProtocol == ">101" || serialStringForProtocol == ">103" || serialStringForProtocol == ">102" || serialStringForProtocol == ">104" || serialStringForProtocol == ">109")) // проверка подлинности при отключении реле
				{
					//7E A0 2C 02 21 61 32 61 6E E6 E6 00 C3 01 C1 00 0F 00 00 28 00 00 FF 01 01 09 10 5A F9 3E D6 AA A0 0C 7F C5 6E 15 D6 88 60 D8 C1 B0 0E 7E

					QByteArray hexValue1 = "\x7E\xA0\x2C\x02\x21\x61\x32\x61\x6E\xE6\xE6";
					QByteArray hexValue2 = "\xC3\x01\xC1";
					QByteArray hexValue3 = "\x0F";
					QByteArray hexValue4 = "\x28";
					QByteArray hexValue5 = "\xFF\x01\x01\x09\x10\x5A\xF9\x3E\xD6\xAA\xA0\x0C\x7F\xC5\x6E\x15\xD6\x88\x60\xD8\xC1\xB0\x0E\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 3)
				{
					//7E A0 1A 02 21 61 54 18 87 E6 E6 00 C0 01 C1 00 46 00 00 60 03 0A FF 04 00 FE 31 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x61\x54\x18\x87\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x46";
					QByteArray hexValue4 = "\x60\x03\x0A\xFF\x04";
					QByteArray hexValue5 = "\xFE\x31\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 4 && (serialStringForProtocol == "_101" || serialStringForProtocol == "_103" || serialStringForProtocol == "_102" || serialStringForProtocol == "_104" || serialStringForProtocol == "_109")) //включение реле
				{
					//7E A0 1C 02 21 61 76 90 BE E6 E6 00 C3 01 C1 00 46 00 00 60 03 0A FF 02 01 0F 00 A5 83 7E

					QByteArray hexValue1 = "\x7E\xA0\x1C\x02\x21\x61\x76\x90\xBE\xE6\xE6";
					QByteArray hexValue2 = "\xC3\x01\xC1";
					QByteArray hexValue3 = "\x46";
					QByteArray hexValue4 = "\x60\x03\x0A\xFF\x02\x01\x0F";
					QByteArray hexValue5 = "\xA5\x83\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 4 && (serialStringForProtocol == ">101" || serialStringForProtocol == ">103" || serialStringForProtocol == ">102" || serialStringForProtocol == ">104" || serialStringForProtocol == ">109")) // отключение реле
				{
					//7E A0 1C 02 21 61 76 90 BE E6 E6 00 C3 01 C1 00 46 00 00 60 03 0A FF 01 01 0F 00 68 A6 7E

					QByteArray hexValue1 = "\x7E\xA0\x1C\x02\x21\x61\x76\x90\xBE\xE6\xE6";
					QByteArray hexValue2 = "\xC3\x01\xC1";
					QByteArray hexValue3 = "\x46";
					QByteArray hexValue4 = "\x60\x03\x0A\xFF\x01\x01\x0F";
					QByteArray hexValue5 = "\x68\xA6\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (reTransmitQuery >= 4)
				{
					counterForResend = 5;
					answerString += "\nNo, stopped or incorrect responses from remote socket";
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

void TcpClient::exchangeFromTimer()
{
	++reTransmitQuery;
	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
		vecExchange();
	else
		exchange();
}

void TcpClient::vecExchange()
{
	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
	{
		if (counterForResend != 13)
		{
			QTimer::singleShot(500, [this]() {

				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				if (counterForResend == 0) // start 1
				{
					//7E A0 21 02 21 41 93 A5 85 81 80 14 05 02 08 00 06 02 08 00 07 04 00 00 00 07 08 04 00 00 00 07 B3 C6 7E

					QByteArray hexValue1 = "\x7E\xA0\x21\x02\x21\x41\x93\xA5\x85\x81\x80\x14\x05\x02\x08";
					QByteArray hexValue2 = "\x06\x02\x08";
					QByteArray hexValue3 = "\x07\x04";
					QByteArray hexValue4 = "\x07\x08\x04";
					QByteArray hexValue5 = "\x07\xB3\xC6\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + nullVal + hexValue4 + nullVal + nullVal + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 1) // start2
				{
					//7E A0 45 02 21 41 10 95 BF E6 E6 00 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 30 30 30 30 30 30 30 30 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 20 1E 5D FF FF BB 9B 7E

					QByteArray hexValue1 = "\x7E\xA0\x45\x02\x21\x41\x10\x95\xBF\xE6\xE6";
					QByteArray hexValue2 = "\x60\x36\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x01\xAC\x0A\x80\x08\x30\x30\x30\x30\x30\x30\x30\x30\xBE\x10\x04\x0E\x01";
					QByteArray hexValue3 = "\x06\x5F\x1F\x04";
					QByteArray hexValue4 = "\x20\x1E\x5D\xFF\xFF\xBB\x9B\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + hexValue4;

					sendMessage(testArray);
				}

				if (counterForResend == 2) // serial
				{
					//7ea01a022141984ba8e6e600c001c100010000600100ff020089a07e

					QByteArray hexValue1 = "\x7e\xa0\x1a\x02\x21\x41\x98\x4b\xa8\xe6\xe6";
					QByteArray hexValue2 = "\xc0\x01\xc1";
					QByteArray hexValue3 = "\x01";
					QByteArray hexValue4 = "\x60\x01";
					QByteArray hexValue5 = "\xff\x02";
					QByteArray hexValue6 = "\x89\xa0\x7e";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				/*
				if (counterForResend == 3) //Type
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 C1 00 01 00 00 60 01 01 FF 02 00 32 BC 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x01";
					QByteArray hexValue4 = "\x60\x01\x01\xFF\x02";
					QByteArray hexValue5 = "\x32\xBC\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}
				*/

				if (counterForResend == 3) //version
				{
					//7E A0 1A 02 21 41 32 1B A2 E6 E6 00 C0 01 C1 00 01 00 00 00 02 01 FF 02 00 4F 26 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x32\x1B\xA2\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x01";
					QByteArray hexValue4 = "\x02\x01\xFF\x02";
					QByteArray hexValue5 = "\x4F\x26\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + nullVal + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 4) // I (phase 1)
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 33 07 00 FF 02 00 53 5F 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x33\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x53\x5F\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 5) // I (phase 2)
				{
					//7E A0 1A 02 21 41 76 3B A6 E6 E6 00 C0 01 41 00 03 01 00 33 07 00 FF 02 00 53 5F 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x76\x3B\xA6\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x33\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\x53\x5F\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 6) // I (phase 3)
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 41 00 03 01 00 47 07 00 FF 02 00 FE 8A 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\x41";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x47\x07";
					QByteArray hexValue5 = "\xFF\x02";
					QByteArray hexValue6 = "\xFE\x8A\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5 + nullVal + hexValue6;

					sendMessage(testArray);
				}

				if (counterForResend == 7) //fUab
				{
					//7E A0 1A 02 21 41 54 2B A4 E6 E6 00 C0 01 C1 00 03 01 00 51 07 0A FF 02 00 01 9E 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x54\x2B\xA4\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x0A\xFF\x02";
					QByteArray hexValue5 = "\x01\x9E\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 8) // fUac
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 C1 00 03 01 00 51 07 02 FF 02 00 D9 7B 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x02\xFF\x02";
					QByteArray hexValue5 = "\xD9\x7B\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 9) //fUbc
				{
					//7E A0 1A 02 21 41 DC 6B AC E6 E6 00 C0 01 C1 00 03 01 00 51 07 15 FF 02 00 59 EF 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\xDC\x6B\xAC\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x15\xFF\x02";
					QByteArray hexValue5 = "\x59\xEF\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 10) //fUIa
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 C1 00 03 01 00 51 07 04 FF 02 00 43 30 7E 

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x04\xFF\x02";
					QByteArray hexValue5 = "\x43\x30\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 11) //fUIb
				{
					//7E A0 1A 02 21 41 98 4B A8 E6 E6 00 C0 01 C1 00 03 01 00 51 07 0F FF 02 00 56 F0 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x98\x4B\xA8\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x0F\xFF\x02";
					QByteArray hexValue5 = "\x56\xF0\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (counterForResend == 12) //fUIc
				{
					//7E A0 1A 02 21 41 10 0B A0 E6 E6 00 C0 01 C1 00 03 01 00 51 07 1A FF 02 00 A0 5D 7E

					QByteArray hexValue1 = "\x7E\xA0\x1A\x02\x21\x41\x10\x0B\xA0\xE6\xE6";
					QByteArray hexValue2 = "\xC0\x01\xC1";
					QByteArray hexValue3 = "\x03\x01";
					QByteArray hexValue4 = "\x51\x07\x1A\xFF\x02";
					QByteArray hexValue5 = "\xA0\x5D\x7E";

					QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + hexValue3 + nullVal + hexValue4 + nullVal + hexValue5;

					sendMessage(testArray);
				}

				if (reTransmitQuery >= 4)
				{
					counterForResend = 13;
					answerString += "No or stopped responses from remote socket. Maybe soft version less then 1.4.15";
					emit messageError();
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
}



void TcpClient::getDaily()
{
	if (counterForResend != 4)
	{
		QTimer::singleShot(500, [this]() {

			QString hexValueZero = QString::number(0, 16);
			QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

			if (counterForResend == 0) //start 1
			{
				//7E A0 1F 02 21 41 93 CC 30 81 80 12 05 01 F0 06 01 F0 07 04 00 00 00 01 08 04 00 00 00 01 F9 CB 7E

				QByteArray hexValue1 = "\x7e\xa0\x1f\x02\x21\x41\x93\xcc\x30\x81\x80\x12\x05\x01\xf0\x06\x01\xf0\x07\x04";
				QByteArray hexValue2 = "\x01\x08\x04";
				QByteArray hexValue3 = "\x01\xf9\xcb\x7e";

				QByteArray testArray = hexValue1 + nullVal + nullVal + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3;

				sendMessage(testArray);
			}

			if (counterForResend == 1) //start 2
			{
				//7E A0 45 02 21 41 10 95 BF E6 E6 00 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 30 30 30 30 30 30 30 30 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F FF FF 00 D1 7E
				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				QByteArray hexValue1 = "\x7E\xA0\x45\x02\x21\x41\x10\x95\xBF\xE6\xE6";

				QByteArray hexValue2 = "\x60\x36\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x01\xAC\x0A\x80\x08\x30\x30\x30\x30\x30\x30\x30\x30\xBE\x10\x04\x0E\x01";
				QByteArray hexValue3 = "\x06\x5F\x1F\x04";
				QByteArray hexValue4 = "\x7E\x1F\xFF\xFF";
				QByteArray hexValue5 = "\xD1\x7E";

				//QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;
				//sendMessage(testArray);
				sendMessage(QByteArray::fromHex(QByteArray("7EA0450221411095BFE6E6006036A1090607608574050801018A0207808B0760857405080201AC0A80083030303030303030BE10040E01000000065F1F0400007E1FFFFF00D17E")));
			}

			if (counterForResend == 2) // daily
			{
				QString firstDayStr = hexDateFunc(dailyArchiveString);
				int tempInt = dailyArchiveString.toInt();
				QString secondDayStr = hexDateFunc(QString::number(tempInt));

				QByteArray firstDayBytes = QByteArray::fromHex(firstDayStr.toUtf8());
				QByteArray secondDayBytes = QByteArray::fromHex(secondDayStr.toUtf8());

				// Фиксированные части кадра (без флагов 7E и CRC)
				QByteArray prefix = QByteArray::fromHex("A04D0221411491A3E6E600C001C100070100620200FF0201010204020412000809060000010000FF0F02120000090C");
				QByteArray middle = QByteArray::fromHex("FF000000FF000000090C");
				QByteArray suffix = QByteArray::fromHex("FF000000FF0000000100");

				// Собираем тело пакета для расчёта CRC (все байты от A0 до последнего 00)
				QByteArray body = prefix + firstDayBytes + middle + secondDayBytes + suffix;

				// Вычисляем CRC-16/KERMIT
				quint16 crc = crc16Kermit(body);
				QByteArray crcBytes;
				crcBytes.append(char(crc & 0xFF));       // младший байт (little-endian)
				crcBytes.append(char((crc >> 8) & 0xFF));

				// Полный кадр с обрамлением 0x7E
				QByteArray fullFrame = QByteArray::fromHex("7E") + body + crcBytes + QByteArray::fromHex("7E");

				sendMessage(fullFrame);
			}

			if (counterForResend == 3) // end
			{
				//7E A0 08 02 21 41 53 5C 72 7E

				QByteArray hexValue1 = "\x7E\xA0\x08\x02\x21\x41\x53\x5C\x72\x7E";
				QByteArray testArray = hexValue1;

				sendMessage(testArray);
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 4;
				answerString += "No or stopped responses from remote socket. Maybe soft version less then 1.4.15 or maybe havent daily archive on query date";
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



void TcpClient::getCurr()
{
	if (counterForResend != 6)
	{
		QTimer::singleShot(500, [this]() {

			QString hexValueZero = QString::number(0, 16);
			QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

			if (counterForResend == 0) //start 1
			{
				//7E A0 1F 02 21 41 93 CC 30 81 80 12 05 01 F0 06 01 F0 07 04 00 00 00 01 08 04 00 00 00 01 F9 CB 7E

				QByteArray hexValue1 = "\x7e\xa0\x1f\x02\x21\x41\x93\xcc\x30\x81\x80\x12\x05\x01\xf0\x06\x01\xf0\x07\x04";
				QByteArray hexValue2 = "\x01\x08\x04";
				QByteArray hexValue3 = "\x01\xf9\xcb\x7e";

				QByteArray testArray = hexValue1 + nullVal + nullVal + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3;

				sendMessage(testArray);
			}

			if (counterForResend == 1) //start 2
			{
				//7E A0 45 02 21 41 10 95 BF E6 E6 00 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 30 30 30 30 30 30 30 30 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F FF FF 00 D1 7E
				QString hexValueZero = QString::number(0, 16);
				QByteArray nullVal = QByteArray::fromHex(hexValueZero.toUtf8());

				QByteArray hexValue1 = "\x7E\xA0\x45\x02\x21\x41\x10\x95\xBF\xE6\xE6";

				QByteArray hexValue2 = "\x60\x36\xA1\x09\x06\x07\x60\x85\x74\x05\x08\x01\x01\x8A\x02\x07\x80\x8B\x07\x60\x85\x74\x05\x08\x02\x01\xAC\x0A\x80\x08\x30\x30\x30\x30\x30\x30\x30\x30\xBE\x10\x04\x0E\x01";
				QByteArray hexValue3 = "\x06\x5F\x1F\x04";
				QByteArray hexValue4 = "\x7E\x1F\xFF\xFF";
				QByteArray hexValue5 = "\xD1\x7E";

				//QByteArray testArray = hexValue1 + nullVal + hexValue2 + nullVal + nullVal + nullVal + hexValue3 + nullVal + nullVal + hexValue4 + nullVal + hexValue5;
				//sendMessage(testArray);
				sendMessage(QByteArray::fromHex(QByteArray("7EA0450221411095BFE6E6006036A1090607608574050801018A0207808B0760857405080201AC0A80083030303030303030BE10040E01000000065F1F0400007E1FFFFF00D17E")));
			}

			if (counterForResend == 2) // daily
			{
				//7EA01A022141DC6BACE6E600C001C100030100010800FF020032687E //summ cur
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141DC6BACE6E600C001C100030100010800FF020032687E")));
			}

			if (counterForResend == 3) // daily
			{
				//7EA01A022141100BA0E6E600C001C100030100010801FF020089747E // day curr
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141100BA0E6E600C001C100030100010801FF020089747E")));
			}

			if (counterForResend == 4) // daily
			{
				//7EA01A022141542BA4E6E600C001C100030100010802FF020044517E // night curr
				sendMessage(QByteArray::fromHex(QByteArray("7EA01A022141542BA4E6E600C001C100030100010802FF020044517E")));
			}

			if (counterForResend == 5) // end
			{
				//7E A0 08 02 21 41 53 5C 72 7E

				QByteArray hexValue1 = "\x7E\xA0\x08\x02\x21\x41\x53\x5C\x72\x7E";
				QByteArray testArray = hexValue1;

				sendMessage(testArray);
			}

			if (reTransmitQuery >= 4)
			{
				counterForResend = 6;
				answerString += "No or stopped responses from remote socket. Maybe soft version less then 1.4.15";
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



void TcpClient::summAnswervector(QString& any)
{
	if (serialStringForProtocol == "*101" || serialStringForProtocol == "*102" || serialStringForProtocol == "*103" || serialStringForProtocol == "*104" || serialStringForProtocol == "*106" || serialStringForProtocol == "*109")
	{
		bool ok;
		bool minus = false;

		QString temporaryAnswer;

		if (counterForResend > 3 && (any.length() == 96)) // защита от сдвоенного ответа
			temporaryAnswer = any.sliced(84);
		else
			temporaryAnswer = any.sliced(36);

		temporaryAnswer.chop(6);
		QString frankenshteinString;

		qDebug() << "after sliced and chop = " + temporaryAnswer << '\n';

		if (counterForResend > 3)
		{
			temporaryAnswer = QString("%1")
				.arg(temporaryAnswer.toULongLong(&ok, 16));
		}

		if (counterForResend == 4 || counterForResend == 5 || counterForResend == 6)
		{
			if (temporaryAnswer.length() == 2)
			{
				temporaryAnswer.push_front("0");
			}

			if (temporaryAnswer.length() == 1)
			{
				temporaryAnswer.push_front("0");
				temporaryAnswer.push_front("0");
			}

			frankenshteinString = temporaryAnswer.last(3);
			temporaryAnswer.chop(3);

			if (temporaryAnswer == "")
			{
				frankenshteinString.push_front(",");
				frankenshteinString.push_front("0");
			}
			else {
				frankenshteinString.push_front(",");
				frankenshteinString.push_front(temporaryAnswer);
			}
		}

		switch (counterForResend)
		{
		case 2:
		{
			answerString += "Serial ";
			break;
		}
		case 3:
		{
			answerString += "\nSoftVer ";
			break;
		}
		case 4:
		{
			answerString += "\nI L1 ";
			break;
		}
		case 5:
		{
			answerString += "\nI L2 ";
			break;
		}
		case 6:
		{
			answerString += "\nI L3 ";
			break;
		}
		case 7:
		{
			answerString += "\nfUab ";
			break;
		}
		case 8:
		{
			answerString += "\nfUac ";
			break;
		}
		case 9:
		{
			answerString += "\nfUbc ";
			break;
		}
		case 10:
		{
			answerString += "\nfUIa ";
			break;
		}
		case 11:
		{
			answerString += "\nfUIb ";
			break;
		}
		case 12:
		{
			answerString += "\nfUIc ";
			break;
		}
		break;
		};

		if (counterForResend > 3)
		{
			if (counterForResend == 4 || counterForResend == 5 || counterForResend == 6)
				answerString += frankenshteinString + ' ';
			else
				answerString += temporaryAnswer + ' ';
		}
		else
		{
			for (auto& val : temporaryAnswer)
			{
				frankenshteinString += val;

				if (frankenshteinString.length() == 2)
				{
					answerString += QString(QByteArray::fromHex(frankenshteinString.toLatin1()));
					frankenshteinString = "";
				}
			}
		}

		if (counterForResend == 4 || counterForResend == 5 || counterForResend == 6)
			qDebug() << "after convert " + frankenshteinString << '\n';
		else
			qDebug() << "after convert " + temporaryAnswer << '\n';
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



void TcpClient::setDailyArchive(QString temp)
{
	dailyArchiveString = temp;
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