#include <QApplication>
#include <iostream>
#include <QThread>
#include <thread>
#include <control.h>
#include <debug.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Control control;
    CUR;
    control.init();
    CUR;
    control.pushStream();
    CUR;

    return a.exec();
}
