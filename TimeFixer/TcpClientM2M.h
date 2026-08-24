

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTextStream>
#include <windows.h>
#include <qdatetime.h>
#include <QtEndian>
#include <cstdlib>

class TcpClientM2M : public QObject
{
	Q_OBJECT

public:
	explicit TcpClientM2M(QObject* parent = nullptr);

	~TcpClientM2M();

	void connectToSavedHost();
	void sendMessage(const QByteArray& message);

	void startConnectToHost(QString any, QString port);
	void stopConnectionWithHost();

	void changeTimeM2M();
	void exchangeFromTimer();
	void checkDateTimeFromDevice(QString rxString);


signals:
	void messageReceived(const int64_t&);
	void messageError();
	void finishM2M();

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

	bool connectedState = false;
	int reTransmitQuery = 0;
	int counterForResend = 0;
	int reConnectCounter = 0;
	int typeDifferent = 0; // 0 - -900    1 - +900   2 - -213   3 - +228   4 - -51   5 - +38     9 - too much
	int codeForCorrect = 9;
};