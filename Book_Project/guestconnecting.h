#ifndef GUESTCONNECTING_H
#define GUESTCONNECTING_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QCloseEvent>

namespace Ui {
class guestConnecting;
}

class guestConnecting : public QDialog
{
    Q_OBJECT

public:
    explicit guestConnecting(QWidget *parent = nullptr);

    ~guestConnecting();

private slots:

    void on_pushButton_accept_clicked();

    void on_pushButton_reject_clicked();

    void on_radioButton_log_clicked();

    void on_radioButton_reg_clicked();

    void adjustTextColor();

    void on_checkBox_pressed();

signals:
    void signal1(QString &mail);
    void signal2(bool admin);

private:
    Ui::guestConnecting *ui;
    QSqlDatabase db;
};

#endif // GUESTCONNECTING_H
