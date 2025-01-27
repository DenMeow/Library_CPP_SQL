#include "mainwindow.h"

#include <QApplication>
#include <guestconnecting.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    guestConnecting dlg;
    QObject::connect(&dlg, &guestConnecting::signal1, &w, &MainWindow::receiveMail);
    QObject::connect(&dlg, &guestConnecting::signal2, &w, &MainWindow::receiveAdmin);

    if(dlg.exec() == QDialog::Rejected)
        return 0;

    w.show();
    return a.exec();
}
