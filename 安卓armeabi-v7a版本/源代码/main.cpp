#include <QApplication>
#include <iostream>
#include <QThread>
#include <thread>
#include <control.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Control control;
    control.init();
    control.pushStream();


    return a.exec();
}
