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
	qDebug() << "Drivers: " << QSqlDatabase::drivers();
	qDebug() << "LibraryDriverPath: " << QCoreApplication::libraryPaths();

	mw_db = QSqlDatabase::addDatabase("QODBC", "TimeFixerConnection");

	mw_db.setDatabaseName("DRIVER={SQL Server};SERVER=10.86.142.14;DATABASE=ProSoft_ASKUE;UID=solexp;PWD=RootToor#;");

	if (!mw_db.open())
	{
		mw_db.lastError().databaseText(); // если что-то пойдёт не так то пишем это в переменные
		mw_db.lastError().driverText();

		qDebug() << '\n' << "Can't open database: " + mw_db.lastError().text() << '\n';
	}
	else
		qDebug() << '\n' << mw_db.connectionName() + " is OPEN" << '\n';
}



void dataBaseCLass::getDeviceParams()
{
	connectDataBase();

	QSqlQuery query(mw_db);
	QString queryString = QString(R"(
SELECT DISTINCT U.Name as 'Название точки в консоли', U.URL as 'IP и Port', U.PhoneNum as 'CSD', U.NumUSD as 'Сетевой адрес', 

 SUBSTRING(
    A.Info,
    CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
) AS 'Отклонение времени в секундах',

 SUBSTRING(
    A.Info,
    CHARINDEX('SerNum=', A.Info) + LEN('SerNum='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('SerNum=', A.Info)) - (CHARINDEX('SerNum=', A.Info) + LEN('SerNum='))
) AS 'Серийный номер',

 SUBSTRING(
    A.Info,
    CHARINDEX('SoftVer=', A.Info) + LEN('SoftVer='),

    CHARINDEX(CHAR(10), A.Info, CHARINDEX('SoftVer=', A.Info)) - (CHARINDEX('SoftVer=', A.Info) + LEN('SoftVer='))
) AS 'Версия ПО'

  FROM AutoInfo as A
  JOIN USD as U on A.ID_USPD = U.ID_USPD
WHERE TypeInfo = 'USPD'
  AND DT >= DATEADD(day, -1, CAST(GETDATE() AS DATE))
  AND Info LIKE '%TimeDiff=%'  -- Убеждаемся что строка содержит TimeDiff
  AND SUBSTRING(A.Info, 
                CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='), 
                CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - 
                (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
    ) NOT LIKE '%[a-zA-Zа-яА-Я]%'
	AND     LEN(SUBSTRING(A.Info, 
                  CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='), 
                  CHARINDEX(CHAR(10), A.Info, CHARINDEX('TimeDiff=', A.Info)) - 
                  (CHARINDEX('TimeDiff=', A.Info) + LEN('TimeDiff='))
    )) -1 >= 8

ORDER BY 4 DESC -- Сортировка по 4-й колонке (Отклонение времени в секундах)
)");

	if (!query.exec(queryString) || !query.next())
	{
		if (query.lastError().isValid())
			qDebug() << "Error in dataBaseCLass::getDeviceParams() when try to get all device with time diff" << '\n' << "Error: " << query.lastError().text();
		else
			qDebug() << "dataBaseCLass::getDeviceParams() is no get devices with diff";
	}
	else
	{
		do
		{
			//qDebug() << query.value(0).toString() << query.value(1).toString() << query.value(2).toString() << query.value(3).toString() << query.value(4).toString() << query.value(5).toString() << query.value(6).toString();
			emit deviceParams(removeSimbols(query.value(0).toString()), removeSimbols(query.value(1).toString()), removeSimbols(query.value(2).toString()), removeSimbols(query.value(3).toString()), removeSimbols(query.value(4).toString()), removeSimbols(query.value(5).toString()), removeSimbols(query.value(6).toString()));
		} while (query.next());
	}

	query.clear();
	mw_db.close();

	emit showArray();
}



QString dataBaseCLass::removeSimbols(QString temp)
{
	temp.remove("\r");
	temp.remove("\n");

	return temp;
}
