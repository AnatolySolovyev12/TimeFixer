#include <QtCore/QCoreApplication>
#include <listClassForHosts.h>
#include "dataBaseCLass.h"
#include "hostsFromDataBase.h"


int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	
	listClassForHosts* hostsList = new listClassForHosts(nullptr);
	
	//hostsFromDataBase* hostsDataBase = new hostsFromDataBase(nullptr);
	//dataBaseCLass* dBclass = new dataBaseCLass(nullptr);

	//QObject::connect(dBclass, &dataBaseCLass::deviceParams, hostsDataBase, &hostsFromDataBase::pushHostInArr);

	//dBclass->getDeviceParams();

	return app.exec();
}