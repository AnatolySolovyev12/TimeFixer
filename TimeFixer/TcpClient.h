

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

	void connectToSavedHost();
	void sendMessage(const QByteArray& message);
	QString returnResultString();
	void setResultString(QString any);
	void startConnectToHost(QString any, QString port);
	void resetAnswerString();
	void setKey(int64_t any);
	const int64_t getKey();
	QString getSerialStringForProtocol();

	QString hexDateFunc(QString date);
	quint16 crc16Kermit(const QByteArray& data);

	void changeTimeArt();
	QByteArray modbusCRCforArtTime(QString temp);
	QByteArray modbusCRCforArtDate(QString temp);
	void stopConnectionWithHost();


signals:
	void messageReceived(const int64_t&);
	void messageError();
	void stopConnection();

private slots:
	void onConnected();
	void onDisconnected();
	void onReadyRead();
	void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
	QTcpSocket* socket;

	bool connectedState = false;
	QTimer* myTimer;
	int counterForResend = 0;
	QString answerString;
	QString m_ip = "";
	QString m_port = "";
	int reTransmitQuery = 0;
	QString serialStringForProtocol;
	int64_t key = 0;
	QString dailyArchiveString = "";
};