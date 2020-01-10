#include "extracttoolwindow.h"
#include "ui_extracttoolwindow.h"

ExtractToolWindow::ExtractToolWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ExtractToolWindow)
{
    ui->setupUi(this);
}

ExtractToolWindow::~ExtractToolWindow()
{
    delete ui;
}


//
//功能:提取文件中的NMEA报文数据
//
void ExtractToolWindow::on_ExtractButton_clicked()
{
#if defined (Q_OS_WIN)
    qDebug()<<"当前是windows操作系统";

    //type 20191112_TEST.nmea | findstr /s /i /c:GGA /c:RMC >tmp.nmea

#elif defined (Q_OS_LINUX)
    qDebug()<<"当前是linux操作系统";
#endif

}
