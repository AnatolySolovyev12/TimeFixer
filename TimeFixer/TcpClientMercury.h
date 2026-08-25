#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTextStream>
#include <windows.h>
#include <qdatetime.h>
#include <QtEndian>
#include <cstdlib>

class TcpClientMercury : public QObject
{
	Q_OBJECT

public:
	explicit TcpClientMercury(QString serial, QObject* parent = nullptr);

	~TcpClientMercury();

	void connectToSavedHost();
	void sendMessage(const QByteArray& message);

	void startConnectToHost(QString any, QString port);
	void stopConnectionWithHost();

	void changeTimeMercury();
	void exchangeFromTimer();
	void checkDateTimeFromDevice(QString rxString);
	quint16 calculateCRC16(const QByteArray& data);
	QByteArray createPacket(const QByteArray& data);



signals:
	void messageReceived(const int64_t&);
	void messageError();
	void finishMercury();

private slots:
	void onConnected();
	void onDisconnected();
	void onReadyRead();
	void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
	QTcpSocket* socket;
	QTimer* myTimer = nullptr;
	QString m_ip = "";
	QString m_port = "";
	QString m_serial = 0;
	QString dateForProtocol;
	QString timeForProtocol;


	bool connectedState = false;
	int reTransmitQuery = 0;
	int counterForResend = 0;
	int reConnectCounter = 0;
};