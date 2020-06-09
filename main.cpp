#include <QCoreApplication>
#include "service.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Service ser;
    return a.exec();
}
