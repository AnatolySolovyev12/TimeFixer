#include <QtCore/QCoreApplication>
#include <listClassForHosts.h>



int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	listClassForHosts* host = new listClassForHosts();

	return app.exec();
}