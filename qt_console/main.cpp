#include <QCoreApplication>
#include <stdio.h>
#include "P3_Protocol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf("hello world!\r\n");

    return a.exec();
}
