#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTextStream>
#include <windows.h>
#include <qdatetime.h>
#include <QtEndian>

class TcpClientArt : public QObject
{
	Q_OBJECT

public:
	explicit TcpClientArt(QObject* parent = nullptr);

	~TcpClientArt();

	void connectToSavedHost();
	void sendMessage(const QByteArray& message);

	void startConnectToHost(QString any, QString port);


	void changeDateTime();
	QByteArray modbusCRCforArtTime(QString temp);
	QByteArray modbusCRCforArtDate(QString temp);
	void stopConnectionWithHost();

	void changeTimeM2M();

signals:
	void messageReceived(const int64_t&);
	void messageError();
	void finishART();

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
	QString serialStringForProtocol;
	
	bool connectedState = false;
	int reTransmitQuery = 0;
	int counterForResend = 0;
	int reConnectCounter = 0;
	bool secondArtCommand = false;
	bool artCycleFinished = false;
};