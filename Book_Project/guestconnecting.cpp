#include "guestconnecting.h"
#include <mainwindow.h>
#include "ui_guestconnecting.h"
#include <QDebug>

guestConnecting::guestConnecting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::guestConnecting) //конструктор
{
    ui->setupUi(this);
    ui -> lineEdit_name -> setPlaceholderText("Введите имя...");
    ui -> lineEdit_name->setStyleSheet("background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF;");
    ui -> lineEdit_familia -> setPlaceholderText("Введите фамилию...");
    ui -> lineEdit_familia->setStyleSheet("background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF;");
    ui -> lineEdit_city -> setPlaceholderText("Введите город...");
    ui -> lineEdit_city->setStyleSheet("background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF;");
    ui -> lineEdit_mail -> setPlaceholderText("Введите адрес электронной почты...");
    ui -> lineEdit_mail->setStyleSheet("background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF;");
    ui -> lineEdit_pwd -> setPlaceholderText("Введите пароль...");
    ui -> lineEdit_pwd->setStyleSheet("background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF;");

    //установка фона
    QPixmap bkgnd(":/new/image/src/bcg.jpg");
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);

    //Иконка главного окна
    QIcon icon(":/new/image/src/i.webp");
    this->setWindowIcon(icon);

    db = QSqlDatabase::addDatabase("QPSQL", "guest_connection");
    db.setDatabaseName("postgres");
    db.setHostName("localhost");
    db.setPassword("1234");
    db.setPort(5432);
    db.setUserName("postgres");
    db.open();

    QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
    ui -> lineEdit_mail -> setValidator(new QRegularExpressionValidator(rx, this));
    connect(ui->lineEdit_mail, &QLineEdit::textChanged, this, &guestConnecting::adjustTextColor);
}

guestConnecting::~guestConnecting() //деструктор
{
    delete ui;
}

void guestConnecting::adjustTextColor() //валидация мыла
{
    if(!ui->lineEdit_mail->hasAcceptableInput())
        ui->lineEdit_mail->setStyleSheet("QLineEdit { background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF; color: red;}");
    else
        ui->lineEdit_mail->setStyleSheet("QLineEdit { background-color: #EBDECF;" "border-radius: 10px;" "border: 1px solid #EBDECF; color: black;}");
}

void guestConnecting::on_pushButton_accept_clicked() //обработка нажатия ОК
{
    QString mail;
    QString name;
    QString familia;
    QString city;
    if(ui -> radioButton_reg -> isChecked()) { //регистрация (обычный пользователь)
        name = ui->lineEdit_name->text();
        familia = ui->lineEdit_familia->text();
        city = ui->lineEdit_city->text();
        mail = ui->lineEdit_mail->text();
        QString pwd = ui->lineEdit_pwd->text();
        if(!ui->lineEdit_mail->hasAcceptableInput()){
            QMessageBox::warning(this, "Ошибка","Некорректный почтовый адрес");
            return;
        }

        // Проверка, заполнены ли все поля
        if (name.isEmpty() || familia.isEmpty() || city.isEmpty() || mail.isEmpty() || pwd.isEmpty()) {
            QMessageBox::warning(this, "Не шали", "Все поля обязательны для заполнения.");
            return;
        }

        // Хеширование пароля
        QByteArray hashedPwd = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha512);
        QString hashedPwdStr = QString::fromUtf8(hashedPwd.toHex()); // Преобразуем хеш в строку

        // Проверяем, существует ли уже пользователь с таким именем/почтой
        QSqlQuery checkQuery(db);
        checkQuery.prepare("SELECT COUNT(*) FROM client WHERE familia=:familia AND city=:city AND mail=:mail");
        checkQuery.bindValue(":familia", familia);
        checkQuery.bindValue(":city", city);
        checkQuery.bindValue(":mail", mail);

        if (!checkQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос регистрации: " + checkQuery.lastError().text());
            return;
        }

        int count = 0;
        while (checkQuery.next()) {
            count = checkQuery.value(0).toInt(); // Получаем количество строк
        }

        if (count > 0) {
            QMessageBox::warning(this, "Пользователь уже существует", "Такой пользователь уже зарегистрирован. Авторизируйтесь");
            return;
        }

        // Подготавливаем SQL-запрос для вставки данных
        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO client (name, familia, city, mail, password) VALUES (:name, :familia, :city, :mail, :password)");
        insertQuery.bindValue(":name", name);
        insertQuery.bindValue(":familia", familia);
        insertQuery.bindValue(":city", city);
        insertQuery.bindValue(":mail", mail);
        insertQuery.bindValue(":password", hashedPwdStr); // Используем захешированный пароль

        if (!insertQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + insertQuery.lastError().text());
            return;
        }
    } else {
        if(!ui -> checkBox -> isChecked()) { //авторизация (обычный пользователь)
            mail = ui->lineEdit_mail->text();
            QString pwd = ui->lineEdit_pwd->text();

            if(!ui->lineEdit_mail->hasAcceptableInput()){
                QMessageBox::warning(this, "Ошибка","Некорректный почтовый адрес");
                return;
            }

            if (mail.isEmpty() or pwd.isEmpty()) {
                QMessageBox::warning(this, "Не шали", "Все поля обязательны для заполнения.");
                return;
            }

            // Хешируем введенный пароль
            QByteArray hashedPwd = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha512);
            QString hashedPwdStr = QString::fromUtf8(hashedPwd.toHex());

            // Запрос к базе данных для поиска пользователя по email и хешу пароля
            QSqlQuery query(db);
            query.prepare("SELECT * FROM client WHERE mail = :mail AND password = :hashedPwdStr");
            query.bindValue(":mail", mail);
            query.bindValue(":hashedPwdStr", hashedPwdStr);

            if (!query.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос авторизации: " + query.lastError().text());
                return;
            }

            if (query.next()) {
                qInfo() << "Пользователь найден!";
            } else {
                QMessageBox::information(this, "Ошибка аутентификации","Такой пользователь не найден? Проверьте введенные данные или зарегистрируйтесь");
                return;
            }
        } else { //авторизация (сотрудник)
            mail = ui->lineEdit_mail->text();
            QString pwd = ui->lineEdit_pwd->text();

            // Проверка, заполнены ли все поля
            if (mail.isEmpty() or pwd.isEmpty()) {
                QMessageBox::warning(this, "Не шали", "Все поля обязательны для заполнения.");
                return;
            }

            // Хеширование пароля
            QByteArray hashedPwd = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha512);
            QString hashedPwdStr = QString::fromUtf8(hashedPwd.toHex()); // Преобразуем хеш в строку

            QSqlQuery query(db);
            query.prepare("SELECT * FROM staff WHERE mail = :mail AND password = :hashedPwdStr");
            query.bindValue(":mail", mail);
            query.bindValue(":hashedPwdStr", hashedPwdStr);

            if (!query.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос авторизации: " + query.lastError().text());
                return;
            }

            if (query.next()) {
                qInfo() << "Пользователь найден!";
            } else {
                QMessageBox::information(this, "Ошибка аутентификации","Такой пользователь не найден? Проверьте введенные данные или обратитесь к администратору");
                return;
            }
            emit signal2(true);
        }
    }
    emit signal1(mail);
    done(true);
}

void guestConnecting::on_pushButton_reject_clicked() //обработка нажатия ОТМЕНА
{
    reject();
}

void guestConnecting::on_radioButton_log_clicked()
{
    ui -> lineEdit_name -> hide();
    ui -> lineEdit_familia -> hide();
    ui -> lineEdit_city -> hide();
    ui -> label -> hide();
    ui -> label_2 -> hide();
    ui -> label_3 -> hide();
}

void guestConnecting::on_radioButton_reg_clicked()
{
    ui -> lineEdit_name -> show();
    ui -> lineEdit_familia -> show();
    ui -> lineEdit_city -> show();
    ui -> label -> show();
    ui -> label_2 -> show();
    ui -> label_3 -> show();
}

void guestConnecting::on_checkBox_pressed() //обработка нажатия чекбокса - сотрудника
{
    if(ui -> checkBox -> isChecked()) {
        ui -> radioButton_reg -> setChecked(true);
        ui -> radioButton_reg -> show();
        on_radioButton_reg_clicked();
    } else {
        ui -> radioButton_log -> setChecked(true);
        ui -> radioButton_reg -> hide();
        on_radioButton_log_clicked();
    }
}
