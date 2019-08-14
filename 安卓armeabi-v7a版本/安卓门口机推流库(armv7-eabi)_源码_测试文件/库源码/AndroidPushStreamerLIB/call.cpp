#include "call.h"
#include <QApplication>
#include <control.h>


void call()
{
    int argc=0;
    char **argv;
    QApplication a(argc, argv);
    Control control;
    control.init();
    control.pushStream();

    a.exec();
}
