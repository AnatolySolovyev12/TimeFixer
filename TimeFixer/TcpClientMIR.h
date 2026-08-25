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

class TcpClientMIR : public QObject
{
	Q_OBJECT

public:
	explicit TcpClientMIR(QObject* parent = nullptr);

	~TcpClientMIR();

	void connectToSavedHost();
	void sendMessage(const QByteArray& message);

	void startConnectToHost(QString any, QString port);
	void stopConnectionWithHost();

	void changeTimeMIR();
	void exchangeFromTimer();
	void checkDateTimeFromDevice(QString rxString);
	QByteArray createDateTimeForDevice();
	quint16 calculateCRC16(const QByteArray& data);
	QByteArray createPacket(const QByteArray& data);



signals:
	void messageReceived(const int64_t&);
	void messageError();
	void finishMIR();

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
	QString dateForProtocol;
	QString timeForProtocol;


	bool connectedState = false;
	int reTransmitQuery = 0;
	int counterForResend = 0;
	int reConnectCounter = 0;

};