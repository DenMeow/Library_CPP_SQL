#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QApplication>
#include <QDialog>

#include <guestconnecting.h>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringListModel>
#include <QCompleter>
#include <QDesktopServices>
#include <QUrl>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct User {
    QString client_id;
    QString name;
    QString familia;
    QString mail;
    QString city;
    bool admin = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void receiveMail(const QString &val);
    void receiveAdmin(bool ad);

private slots:    
    void on_pushButton_all_clicked();

    void buy_click();
    void rebuy_click();
    void like_click();
    void dislike_click();
    void block();

    void pick_click();

    void on_pushButton_best_clicked();

    void on_pushButton_rent_clicked();

    void on_pushButton_search_clicked();

    void on_pushButton_filter_clicked();

    void on_pushButton_allUser_clicked();

    void on_pushButton_allRent_clicked();

private:
    Ui::MainWindow *ui;
    void db_connect();

    QSqlDatabase db;
    guestConnecting *dlg;
    User user;
};
#endif // MAINWINDOW_H
