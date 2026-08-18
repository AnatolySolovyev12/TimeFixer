#include "dataBaseCLass.h"

dataBaseCLass::dataBaseCLass(QObject* parent)
	: QObject(parent)
{
}



dataBaseCLass::~dataBaseCLass()
{
}



void dataBaseCLass::connectDataBase()
{
	mw_db = QSqlDatabase::addDatabase("QODBC");

	mw_db.setDatabaseName("DRIVER={SQL Server};SERVER=10.86.142.14;DATABASE=ProSoft_ASKUE;UID=solexp;PWD=RootToor#;");

	if (!mw_db.open()) 
	{
		mw_db.lastError().databaseText(); // если что-то пойдЄт не так то пишем это в переменные
		mw_db.lastError().driverText();

		qDebug() << "Can't open database: " + mw_db.lastError().text();
	}
}



void dataBaseCLass::getDeviceParams()
{
	connectDataBase();

	QSqlQuery query;
	QString queryString = QString(R"(
SELECT DISTINCT U.Name as 'name', U.URL as 'IP и Port', U.PhoneNum as 'CSD', U.NumUSD as 'network ip', 

 SUBSTRING(
    A.Info,
    CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
) AS 'TimeDiff',

 SUBSTRING(
    A.Info,
    CHARINDEX('SerNum=', A.Info) + LEN('SerNum='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('SerNum=', A.Info)) - (CHARINDEX('SerNum=', A.Info) + LEN('SerNum='))
) AS 'Serial',

 SUBSTRING(
    A.Info,
    CHARINDEX('SoftVer=', A.Info) + LEN('SoftVer='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('SoftVer=', A.Info)) - (CHARINDEX('SoftVer=', A.Info) + LEN('SoftVer='))
) AS 'SoftVer'

  FROM AutoInfo as A
  JOIN USD as U on A.ID_USPD = U.ID_USPD
WHERE TypeInfo = 'USPD'
  AND DT >= DATEADD(day, -1, CAST(GETDATE() AS DATE))
  AND Info LIKE '%TimeDiff=%'
  AND SUBSTRING(A.Info, 
                CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='), 
                CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - 
                (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
    ) NOT LIKE '%[a-zA-Zа-€ј-я]%'
	AND     LEN(SUBSTRING(A.Info, 
                  CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='), 
                  CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - 
                  (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
    )) -1 >= 8

ORDER BY 4 DESC
)");

	if (!query.exec() || !query.next())
	{
		if(query.lastError().isValid())
			qDebug() << "Error in dataBaseCLass::getDeviceParams() when try to get all device with time diff.\nError: " + query.lastError().text();
		else
			qDebug() << "dataBaseCLass::getDeviceParams() is no get devices with diff";
	}
	else
	{
		do
		{
			emit deviceParams(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toString(), query.value(5).toString());
		} while (query.next());
	}

	mw_db.close();

	mw_db.removeDatabase(QSqlDatabase::defaultConnection);
	resultBool = false;
}
