#include "extracttoolwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ExtractToolWindow w;
    w.show();

    return a.exec();
}
