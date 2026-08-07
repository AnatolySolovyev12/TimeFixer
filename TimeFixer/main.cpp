#include <QtCore/QCoreApplication>

#include "TcpClient.h"
#include <qdatetime.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TcpClient test("test");
    test.connectToServer("10.0.5.136", 40001);

    return app.exec();
}
