#include "dataBaseClass.h"

dataBaseClass::dataBaseClass(QObject* parent)
	: QObject(parent)
{
	AttachConsole(ATTACH_PARENT_PROCESS);
	readDataBaseFile();
}



dataBaseClass::~dataBaseClass()
{
}



void dataBaseClass::connectDataBase()
{
	qDebug() << "Drivers: " << QSqlDatabase::drivers();
	qDebug() << "LibraryDriverPath: " << QCoreApplication::libraryPaths();

	mw_db = QSqlDatabase::addDatabase("QODBC", "TimeFixerConnection");

	mw_db.setDatabaseName("DRIVER={SQL Server};SERVER=" + host + ";DATABASE=" + dataBase + ";UID=" + login + ";PWD=" + pass + ";");

	if (!mw_db.open())
	{
		mw_db.lastError().databaseText(); // если что-то пойдёт не так то пишем это в переменные
		mw_db.lastError().driverText();

		qDebug() << '\n' << "Can't open database: " + mw_db.lastError().text() << '\n';
	}
	else
		qDebug() << '\n' << mw_db.connectionName() + " is OPEN" << '\n';
}



void dataBaseClass::getDeviceParams()
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
			qDebug() << "Error in dataBaseClass::getDeviceParams() when try to get all device with time diff" << '\n' << "Error: " << query.lastError().text();
		else
			qDebug() << "dataBaseClass::getDeviceParams() is no get devices with diff";
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



QString dataBaseClass::removeSimbols(QString temp)
{
	temp.remove("\r");
	temp.remove("\n");

	return temp;
}



bool dataBaseClass::readDataBaseFile()
{
	QFile file(QCoreApplication::applicationDirPath() + "\\dataBase.txt");

	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Don't find dataBase file. Create file and try again";
		return false;
	}

	QTextStream out(&file);

	QString* myLine = new QString();

	int counter = 0;

	while (out.readLineInto(myLine, 0))
	{
		switch (counter)
		{
		case(0):
		{
			host = *myLine;
			break;
		}
		case(1):
		{
			dataBase = *myLine;
			break;
		}
		case(2): 
		{
			login = *myLine;
			break;
		}
		case(3):
		{
			pass = *myLine;
			break;
		}
		}

		++counter;
	}

	delete myLine;
	myLine = nullptr;
	file.close();
}