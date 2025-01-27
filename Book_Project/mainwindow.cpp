#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //Конструктор
{
    db_connect();
    ui->setupUi(this);

    //Картинка главного окна
    QPixmap bkgnd(":/new/image/src/main_window.jpg");
    bkgnd = bkgnd.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);
    //Иконка главного окна
    QIcon icon(":/new/image/src/icon_main.jpg");
    this->setWindowIcon(icon);
    //Картинка Москвы
    QImage image1(":/new/image/src/moscow.jpg");
    QPixmap pixmap1 = QPixmap::fromImage(image1);
    ui -> label_moscow -> setPixmap(pixmap1);
    ui -> label_moscow -> setScaledContents(true);
    //Картинка свечек
    QImage image(":/new/image/src/books.jpg");
    QPixmap pixmap = QPixmap::fromImage(image);
    ui -> label_books -> setPixmap(pixmap);
    ui -> label_books -> setScaledContents(true);

    on_pushButton_all_clicked();

    QStringList suggestions {"Идиот", "Игрок", "Мастер и Маргарита", "Белая гвардия", "Братья Карамазовы", "Стихотворения и поэмы", "Лирика", "Черный человек"};

    QStringListModel* model = new QStringListModel(suggestions);// Создание модели для хранения списка предложений
    QCompleter* completer = new QCompleter(model, this); // Создание объекта QCompleter и передача ему модели
    ui -> lineEdit_nameBook -> setCompleter(completer);// Установка комплитера для нашего LineEdit

    ui -> pushButton_allRent -> hide();
    ui -> pushButton_allUser -> hide();
}

MainWindow::~MainWindow() //Деструктор
{
    db.close();
    delete dlg;
    delete ui;
}

void MainWindow::db_connect() //подключение к Базе Данных
{
    db = QSqlDatabase::addDatabase("QPSQL");

    db.setDatabaseName("postgres");
    db.setHostName("localhost");
    db.setPassword("1234");
    db.setPort(5432);
    db.setUserName("postgres");
    if(!db.open()) {
        QMessageBox::critical(this,"Все плохо","Нет подключения к БД");
    }
}

void MainWindow::receiveMail(const QString &mail) //полученные данные о пользователе
{
    if(!user.admin) {
        QSqlQuery query(db);
        query.prepare("SELECT name, familia, city, client_id FROM client WHERE mail = :mail");
        query.bindValue(":mail", mail);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось получить данные из базы.");
            return;
        }
        if (query.next()) {
            // Извлекаем значения из результата запроса
            user.name = query.value(0).toString();
            user.familia = query.value(1).toString();
            user.city = query.value(2).toString();
            user.client_id = query.value(3).toString();
            user.mail = mail;

            ui->lineEdit_name->setText(user.name);
            ui->lineEdit_familia->setText(user.familia);
        }
    } else {
        user.name = "admin";
        user.familia = "admin";
        user.mail = mail;

        ui->lineEdit_name->setText(user.name);
        ui->lineEdit_familia->setText(user.familia);
    }

}

void MainWindow::receiveAdmin(bool ad) //подключение админа
{
    user.admin = ad;
    ui -> pushButton_allRent -> show();
    ui -> pushButton_allUser -> show();
    ui -> pushButton_best -> hide();
    ui -> pushButton_rent -> hide();
    on_pushButton_all_clicked();
}

void MainWindow::on_pushButton_all_clicked() //вывести все книги
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id WHERE count > 0 ORDER BY book_id ASC");
    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }

    if(user.admin) {
        ui -> tableWidget ->setColumnCount(columnCount);
    } else {
        headers.append("");
        headers.append("");
        ui -> tableWidget ->setColumnCount(columnCount+2);
    }
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);


            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        if(!user.admin) {
            QPushButton *buy = new QPushButton();
            buy -> setText("Взять");
            buy -> setProperty("row", rowIndex);
            connect(buy, SIGNAL(clicked()), this, SLOT(buy_click()));
            ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);

            QPushButton *like = new QPushButton();
            like -> setText("🩷");
            like -> setProperty("row", rowIndex);
            connect(like, SIGNAL(clicked()), this, SLOT(like_click()));
            ui -> tableWidget ->setCellWidget(rowIndex, columnCount+1, like);
        }
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
    ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
}


void MainWindow::buy_click() //взять книгу
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    if (pb != nullptr) {
        int row = pb->property("row").toInt();

        QString book_id = ui -> tableWidget -> item(row, 0) -> text();

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM buy_book WHERE client_id = :client_id AND book_id = :book_id");
        checkQuery.bindValue(":client_id", user.client_id);
        checkQuery.bindValue(":book_id", book_id);

        if (!checkQuery.exec()) {
            qDebug() << "Ошибка выполнения запроса проверки:" << checkQuery.lastError().text();
            return;
        }

        if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Ошибка!","Эта книга уже у Вас на руках.");
            return;
        } else {
            QSqlQuery insertQuery(db);
            insertQuery.prepare("INSERT INTO buy_book (client_id, book_id) VALUES (:client_id, :book_id)");
            insertQuery.bindValue(":client_id", user.client_id);
            insertQuery.bindValue(":book_id", book_id);

            if (!insertQuery.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + insertQuery.lastError().text());
                return;
            }

            QSqlQuery updateQuery(db);
            updateQuery.prepare("UPDATE book SET count = count - 1 WHERE book_id = :book_id");
            updateQuery.bindValue(":book_id", book_id);

            if (!updateQuery.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + updateQuery.lastError().text());
                return;
            }
        }

        int up = ui -> tableWidget -> item(row, ui -> tableWidget -> columnCount()-3) -> text().toInt();
        ui -> tableWidget -> item(row, ui -> tableWidget -> columnCount()-3) -> setText(QString::number(up-1));
        ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
    }
}

void MainWindow::rebuy_click()  //вернуть книгу
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    int row = pb->property("row").toInt();

    QString book_id = ui -> tableWidget -> item(row, 0) -> text();

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM buy_book WHERE book_id = :book_id AND client_id = :client_id");
    deleteQuery.bindValue(":book_id", book_id);
    deleteQuery.bindValue(":client_id", user.client_id);

    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления строки:" << deleteQuery.lastError().text();
    } else {
        qDebug() << "Строка успешно удалена.";
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE book SET count = count + 1 WHERE book_id = :book_id");
        updateQuery.bindValue(":book_id", book_id);

        if (!updateQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + updateQuery.lastError().text());
            return;
        }
        on_pushButton_rent_clicked();
    }
}


void MainWindow::like_click() //добавить книгу в любимые
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    if (pb != nullptr) {
        int row = pb->property("row").toInt();

        QString book_id = ui -> tableWidget -> item(row, 0) -> text();

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM best WHERE client_id = :client_id AND book_id = :book_id");
        checkQuery.bindValue(":client_id", user.client_id);
        checkQuery.bindValue(":book_id", book_id);

        if (!checkQuery.exec()) {
            qDebug() << "Ошибка выполнения запроса проверки:" << checkQuery.lastError().text();
            return;
        }

        if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Ошибка!","Эта книга уже у Вас в любимых.");
        } else {
            QSqlQuery insertQuery(db);
            insertQuery.prepare("INSERT INTO best (client_id, book_id) VALUES (:client_id, :book_id)");
            insertQuery.bindValue(":client_id", user.client_id);
            insertQuery.bindValue(":book_id", book_id);

            if (!insertQuery.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + insertQuery.lastError().text());
                return;
            }
        }
    }
}

void MainWindow::dislike_click() //убрать книгу из любимых
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    int row = pb->property("row").toInt();

    QString book_id = ui -> tableWidget -> item(row, 0) -> text();

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM best WHERE book_id = :book_id AND client_id = :client_id");
    deleteQuery.bindValue(":book_id", book_id);
    deleteQuery.bindValue(":client_id", user.client_id);

    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления строки:" << deleteQuery.lastError().text();
    } else {
        qDebug() << "Строка успешно удалена.";
        on_pushButton_best_clicked();
    }
}


void MainWindow::on_pushButton_best_clicked() //показать корзину
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT book.book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id JOIN best ON best.book_id = book.book_id WHERE best.client_id = :client_id");
    query.bindValue(":client_id", user.client_id);

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");
    headers.append("");

    ui -> tableWidget ->setColumnCount(columnCount+2);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *buy = new QPushButton();
        buy -> setText("Взять");
        buy -> setProperty("row", rowIndex);
        connect(buy, SIGNAL(clicked()), this, SLOT(buy_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);

        QPushButton *dislike = new QPushButton();
        dislike -> setText("💔");
        dislike -> setProperty("row", rowIndex);
        connect(dislike, SIGNAL(clicked()), this, SLOT(dislike_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount+1, dislike);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
    ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
}

void MainWindow::on_pushButton_rent_clicked() //показать книги на руках
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT book.book_id, title AS Название, genre AS Жанр, name_author AS Автор FROM book  JOIN author ON book.author_id = author.author_id JOIN buy_book ON buy_book.book_id = book.book_id WHERE buy_book.client_id = :client_id");
    query.bindValue(":client_id", user.client_id);

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");

    ui -> tableWidget ->setColumnCount(columnCount+1);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *buy = new QPushButton();
        buy -> setText("Вернуть");
        buy -> setProperty("row", rowIndex);
        connect(buy, SIGNAL(clicked()), this, SLOT(rebuy_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
    ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
}

void MainWindow::on_pushButton_search_clicked() //поиск по фильтрам
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    QString nameBook = ui -> lineEdit_nameBook -> text();

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id WHERE count > 0 AND title = :nameBook ORDER BY book_id ASC");
    query.bindValue(":nameBook", nameBook);
    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");
    headers.append("");
    ui -> tableWidget ->setColumnCount(columnCount+2);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *buy = new QPushButton();
        buy -> setText("Взять");
        buy -> setProperty("row", rowIndex);
        connect(buy, SIGNAL(clicked()), this, SLOT(buy_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);

        QPushButton *like = new QPushButton();
        like -> setText("🩷");
        like -> setProperty("row", rowIndex);
        connect(like, SIGNAL(clicked()), this, SLOT(like_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount+1, like);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
}

void MainWindow::on_pushButton_filter_clicked() //поиск по фильтру
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    QString nameAuthor = ui -> comboBox_nameAuthor -> currentText();
    QString nameGenre = ui -> comboBox_nameGenre -> currentText();

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    if(nameAuthor == "" and nameGenre == "") {
        query.prepare("SELECT book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id WHERE count > 0 ORDER BY book_id ASC");
    } else if(nameGenre.size() > 0 and nameAuthor.size() > 0) {
        query.prepare("SELECT book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id WHERE count > 0 AND name_author = :nameAuthor AND genre = :nameGenre ORDER BY book_id, name_author, genre ASC");
        query.bindValue(":nameAuthor", nameAuthor);
        query.bindValue(":nameGenre", nameGenre);
    } else {
        query.prepare("SELECT book_id, title AS Название, genre AS Жанр, name_author AS Автор, count AS Количество FROM book JOIN author ON book.author_id = author.author_id WHERE count > 0 AND name_author = :nameAuthor OR genre = :nameGenre ORDER BY book_id, name_author, genre ASC");
        query.bindValue(":nameAuthor", nameAuthor);
        query.bindValue(":nameGenre", nameGenre);
    }


    if (!query.exec()) {
        QMessageBox::information(this, "УПС..","Такой книги не найдено(");
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");
    headers.append("");
    ui -> tableWidget ->setColumnCount(columnCount+2);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *buy = new QPushButton();
        buy -> setText("Взять");
        buy -> setProperty("row", rowIndex);
        connect(buy, SIGNAL(clicked()), this, SLOT(buy_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);

        QPushButton *like = new QPushButton();
        like -> setText("🩷");
        like -> setProperty("row", rowIndex);
        connect(like, SIGNAL(clicked()), this, SLOT(like_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount+1, like);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
}


void MainWindow::on_pushButton_allUser_clicked() //показать всех пользователей
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT client_id AS ID, name AS Имя, familia AS Фамилия, mail AS Почта, city AS Город FROM client");

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");

    ui -> tableWidget ->setColumnCount(columnCount+1);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *buy = new QPushButton();
        buy -> setText("Заблокировать");
        buy -> setProperty("row", rowIndex);
        connect(buy, SIGNAL(clicked()), this, SLOT(block()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, buy);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
    ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
}

void MainWindow::block() //блокировка пользователя
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    int row = pb->property("row").toInt();

    QString client_id = ui -> tableWidget -> item(row, 0) -> text();

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM client WHERE client_id = :client_id");
    deleteQuery.bindValue(":client_id", client_id);

    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления строки:" << deleteQuery.lastError().text();
    } else {
        qDebug() << "Строка успешно удалена.";
        on_pushButton_allUser_clicked();
    }
}

void MainWindow::on_pushButton_allRent_clicked() //все выданные книги
{
    // Очистка таблицы перед новым выводом данных
    ui -> tableWidget ->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    // Выполняем запрос для получения всех записей из таблицы 'book'
    QSqlQuery query;
    query.prepare("SELECT book.book_id, client.name AS Имя, client.familia AS Фамилия, title AS Название, genre AS Жанр, name_author AS Автор FROM book  JOIN author ON book.author_id = author.author_id JOIN buy_book ON buy_book.book_id = book.book_id JOIN client ON buy_book.client_id = client.client_id");

    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return;
    }

            // Получаем количество столбцов в таблице
    int columnCount = query.record().count();

            // Настраиваем заголовки таблицы
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers.append(query.record().fieldName(i));
    }
    headers.append("");
    ui -> tableWidget ->setColumnCount(columnCount+1);
    ui -> tableWidget ->setHorizontalHeaderLabels(headers);

            // Заполняем таблицу данными
    int rowIndex = 0;
    while (query.next()) {
        ui -> tableWidget ->insertRow(rowIndex);
        for (int colIndex = 0; colIndex < columnCount; ++colIndex) {
            QVariant value = query.value(colIndex);
            QTableWidgetItem *twi = new QTableWidgetItem();
            twi -> setText(value.toString());
            twi->setFlags(twi->flags()&0xfffffffd);
            ui -> tableWidget ->setItem(rowIndex, colIndex, twi);
        }
        QPushButton *pick = new QPushButton();
        pick -> setText("Забрать");
        pick -> setProperty("row", rowIndex);
        connect(pick, SIGNAL(clicked()), this, SLOT(pick_click()));
        ui -> tableWidget ->setCellWidget(rowIndex, columnCount, pick);
        rowIndex++;
    }

    // Автоматически подстраиваем ширину колонок под содержимое
    ui -> tableWidget ->resizeColumnsToContents();
    ui -> tableWidget -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch); //равномерное заполнение таблицы
}

void MainWindow::pick_click() //забрать книгу
{
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    int row = pb->property("row").toInt();

    QString book_id = ui -> tableWidget -> item(row, 0) -> text();

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM buy_book WHERE book_id = :book_id");
    deleteQuery.bindValue(":book_id", book_id);

    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления строки:" << deleteQuery.lastError().text();
    } else {
        qDebug() << "Строка успешно удалена.";
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE book SET count = count + 1 WHERE book_id = :book_id");
        updateQuery.bindValue(":book_id", book_id);

        if (!updateQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось записать данные в БД: " + updateQuery.lastError().text());
            return;
        }
        on_pushButton_allRent_clicked();
    }
}
