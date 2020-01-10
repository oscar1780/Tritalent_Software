#ifndef EXTRACTTOOLWINDOW_H
#define EXTRACTTOOLWINDOW_H

#include <QMainWindow>
#include <QDebug>

namespace Ui {
class ExtractToolWindow;
}

class ExtractToolWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ExtractToolWindow(QWidget *parent = nullptr);
    ~ExtractToolWindow();

private slots:
    void on_ExtractButton_clicked();

private:
    Ui::ExtractToolWindow *ui;
};

#endif // EXTRACTTOOLWINDOW_H
