﻿#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTextStream>
#include <windows.h>
#include <qdatetime.h>
#include <QtEndian>

class TcpClient : public QObject
{
	Q_OBJECT

public:
	explicit TcpClient(QString any, QObject* parent = nullptr);

	~TcpClient();

	void connectToServer(const QString& host, quint16 port);
	void sendMessage(const QByteArray& message);
	void exchange();
	void summAnswer(QString& any);
	void summAnswervector(QString& any);
	QString returnResultString();
	void setResultString(QString any);
	void startToConnect(QString any, QString port = "8888");
	void resetAnswerString();
	void vecExchange();
	void setKey(int64_t any);
	const int64_t getKey();
	QString getSerialStringForProtocol();
	void getDaily();
	void setDailyArchive(QString temp);
	QString hexDateFunc(QString date);
	quint16 crc16Kermit(const QByteArray& data);
	void getCurr();

signals:
	void messageReceived(const int64_t&);
	void messageError();

private slots:
	void onConnected();
	void onDisconnected();
	void onReadyRead();
	void onErrorOccurred(QAbstractSocket::SocketError socketError);
	void exchangeFromTimer();

private:
	QTcpSocket* socket;

	bool connectedState = false;
	QTimer* myTimer;
	int counterForResend = 0;
	QString answerString;
	QString ip = "";
	//int port = 8888;
	int reTransmitQuery = 0;
	QString serialStringForProtocol;
	int64_t key = 0;
	QString dailyArchiveString = "";
};