#include "formtable.h"
#include "ui_formtable.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QTableView>
#include <globalpath.h>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>
#include <QSqlTableModel>
#include <globalpath.h>
#include <authclass.hpp>
#include <admincheck.h>
#include <QInputDialog>
#include <QSpinBox>
#include <QFormLayout>

FormTable::FormTable(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormTable)
    , deleteButton(nullptr)
{
    ui->setupUi(this);
    connectToDB();
    admincheck w;

    // Создание диалогового окна с полями для ввода логина и пароля
    QDialog dialog(&w);
    dialog.resize(400, 200); // Установка размеров диалогового окна
    QVBoxLayout layout(&dialog);

    QLabel titleLabel("Вход");
    titleLabel.setAlignment(Qt::AlignCenter);
    layout.addWidget(&titleLabel);

    QHBoxLayout loginLayout;
    QLabel loginLabel("Имя пользователя:");
    QLineEdit loginEdit;
    loginLayout.addWidget(&loginLabel);
    loginLayout.addWidget(&loginEdit);

    QHBoxLayout passwordLayout;
    QLabel passwordLabel("Пароль:");
    QLineEdit passwordEdit;
    passwordEdit.setEchoMode(QLineEdit::Password);
    passwordLayout.addWidget(&passwordLabel);
    passwordLayout.addWidget(&passwordEdit);

    QPushButton submitButton("Войти");
    layout.addLayout(&loginLayout);
    layout.addLayout(&passwordLayout);
    layout.addWidget(&submitButton);

    QObject::connect(&submitButton, &QPushButton::clicked, [&]() {
        QString login = loginEdit.text();
        QString password = passwordEdit.text();

        // Проверка логина и пароля
        int isAdmin = checkAdmin(login, password, &w);

        if (isAdmin == 1) {
            // Если администратор
            setIsAdmin("1");
            QMessageBox::information(nullptr, "Успех", "Вы успешно вошли как администратор");
            dialog.close();
            w.show();
        } else if(isAdmin == 0) {
            // Если не администратор
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось войти");
            return;
            return;
        } else if(isAdmin == 2) {
            // Если обычный пользователь
            setIsAdmin("2");
            QMessageBox::information(nullptr, "Успех", "Вы успешно вошли как пользователь");
            dialog.close();
            w.show();
        }
    });

    dialog.exec();





}
int FormTable::checkAdmin(const QString& login, const QString& password, admincheck* w) {
    QSqlQuery checkAdminQuery;
    checkAdminQuery.prepare("SELECT login, password, isAdmin FROM users WHERE login = :login AND password = :password");
    checkAdminQuery.bindValue(":login", login);
    checkAdminQuery.bindValue(":password", password);

    if (!checkAdminQuery.exec()) {
        qDebug() << "Error executing query: " << checkAdminQuery.lastError().text();
        return 0;
    }

    if (checkAdminQuery.next()) {
        QString retrievedLogin = checkAdminQuery.value(0).toString();
        QString retrievedPassword = checkAdminQuery.value(1).toString();
        QString retrievedStatus = checkAdminQuery.value(2).toString();
        if (retrievedLogin == login && retrievedPassword == password && retrievedStatus == "true") {
            return 1;
        } else if(retrievedLogin == login && retrievedPassword == password && retrievedStatus == "false") {
            return 2;
        } else {
            return 0;
        }
    } else {
        return 0; // No matching user found
    }
}

void FormTable::setIsAdmin(const QString& value) {
    if (value == "1") {
        // Handle case when the user is an administrator
        userStatus_ = 1;
        qDebug()<<"it a admin";
    } else if (value == "2") {
        // Handle case when the user is a regular user
        userStatus_ = 2;
        qDebug()<<"it a user";
    } else {
        // Handle other cases if necessary
        qDebug() << "Unknown user status";
    }
}


void FormTable::connectToDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(globalPath::getDatabasePath());
    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка при открытии базы данных: " + db.lastError().text());
        return;
    }

    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table'");
    QHBoxLayout *horizontalLayout = new QHBoxLayout; // Создаем горизонтальный контейнер
    while (query.next()) {
        QString tableName = query.value(0).toString();
        if (tableName != "users" && tableName != "sqlite_sequence") { // Исключаем таблицу 'users' и 'sqlite_sequence'
            QPushButton *button = new QPushButton(tableName);
            horizontalLayout->addWidget(button); // Добавляем кнопку в горизонтальный контейнер
            connect(button, &QPushButton::clicked, this, [this, tableName]() {
                updateTableView(tableName);
            });
        }
    }
    ui->verticalLayout->addLayout(horizontalLayout); // Добавляем горизонтальный контейнер в вертикальный контейнер
}


void FormTable::deleteData() {
    if (userStatus_ == 2) {
        QMessageBox::critical(this, "Access Denied", "You don't have permission to delete data.");
        return;
    }
    QMessageBox::StandardButton confirmDelete = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите удалить выбранную запись?", QMessageBox::Yes | QMessageBox::No);
    if (confirmDelete != QMessageBox::Yes) {
        return;
    }
    QModelIndex index = ui->tableView->selectionModel()->currentIndex();
    int id = index.sibling(index.row(), 0).data().toInt(); // Assuming id is in the first column
    QString tableName2 =   ui->labelTable->text();
    deleteDataById(tableName2, id);
}
void FormTable::deleteDataById(const QString& tableName, int id) {
    if (userStatus_ == 2) {
        QMessageBox::critical(this, "Access Denied", "You don't have permission to delete data.");
        return;
    }
    // Получаем текст из labelTable
   // QString labelText =ui->verticalLayout_3->labelTable->text(); // Преобразуем текст в нижний регистр

    // Создаем SQL-запрос для удаления данных из таблицы с использованием имени таблицы

    // Подготавливаем запрос
    QSqlQuery query;
    QString name_id;
    qDebug() << tableName;
    if(tableName == "Workers") {
        name_id = "worker";
    } else if(tableName == "Patients") {
        name_id = "patient";
    } else if (tableName == "Doctors") {
        name_id = "doctor";
    }
    else {
        name_id = tableName;
    }
    // Формируем строку запроса с использованием имени таблицы и значения id
    QString queryString = "DELETE FROM " + tableName + " WHERE id_" + name_id.toLower() + "=" + QString::number(id);

    qDebug() << "queryString" <<queryString;
    // Подготавливаем запрос с сформированной строкой
    query.prepare(queryString);


    qDebug() << "Binding id:" << id; // Отладочное сообщение для отображения значения id
    QString tableName2 = ui->labelTable->text().toLower();
    qDebug() <<"table"<<tableName2;
    updateTableView(tableName2);
    // Выполняем запрос
    if (query.exec()) {

        QMessageBox::information(this, "Success", "Data with ID " + QString::number(id) + " deleted successfully.");
        // Опционально обновляем представление таблицы после успешного удаления

    } else {
        QMessageBox::critical(this, "Error", "Failed to delete data with ID " + QString::number(id) + ": " + query.lastError().text());
    }
    updateTableView(tableName2);
}



void FormTable::updateData(const QList<QPair<QString, QWidget*>>& inputWidgets) {
    if (userStatus_ == 2) {
        QMessageBox::critical(this, "Access Denied", "You don't have permission to delete data.");
        return;
    }
    QString tableName = ui->labelTable->text().toLower(); // Получаем имя текущей таблицы
    QModelIndex index = ui->tableView->selectionModel()->currentIndex();
    int id = index.sibling(index.row(), 0).data().toInt(); // Assuming id is in the first column

    QStringList fieldNames;
    QStringList fieldValues;

    // Получаем значения из виджетов для ввода данных
    for (const auto& pair : inputWidgets) {
        const QString& fieldName = pair.first;
        const QWidget* widget = pair.second;

        if (const QLineEdit* lineEdit = qobject_cast<const QLineEdit*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(lineEdit->text());
        } else if (const QDateEdit* dateEdit = qobject_cast<const QDateEdit*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(dateEdit->date().toString("yyyy-MM-dd"));
        } else if (const QComboBox* comboBox = qobject_cast<const QComboBox*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(comboBox->currentText());
        }
    }

    // Проверяем, что есть поля для обновления
    if (fieldNames.isEmpty() || fieldValues.isEmpty()) {
        QMessageBox::critical(this, "Error", "No fields to update.");
        return;
    }
    QMessageBox::StandardButton confirmUpdate = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите обновить данные?", QMessageBox::Yes | QMessageBox::No);
    if (confirmUpdate != QMessageBox::Yes) {
        return;
    }


    QString labelText = ui->labelTable->text().toLower(); // Преобразуем текст в нижний регистр

    // Создаем SQL-запрос для обновления данных
    QString name_id = (labelText == "workers") ? "worker" : labelText;
    qDebug() << "Table Label: " << labelText;
    qDebug() << "ID Label: " << name_id;
    qDebug() << "Field Names: " << fieldNames;
    qDebug() << "Field Values: " << fieldValues;

    QString queryString = "UPDATE " + labelText + " SET ";
    for (int i = 0; i < fieldNames.size(); ++i) {
        queryString += fieldNames[i] + " = '" + fieldValues[i] + "'";
        if (i < fieldNames.size() - 1) {
            queryString += ", ";
        }
    }
    queryString += " WHERE id_" + name_id + " = :id";

    // Выполняем SQL-запрос
    QSqlQuery query;
    qDebug() << "SQL Query: " << queryString;

    query.prepare(queryString);
    query.bindValue(":id", id);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Data updated successfully.");
        updateTableView(tableName); // Обновляем представление таблицы
    } else {
        QMessageBox::critical(this, "Error", "Failed to update data: " + query.lastError().text());
    }

    // Закрываем запрос
   // query.finish();
}


// Определение слота для обновления данных

void FormTable::updateTableView(const QString& tableName) {
    // Очистка предыдущих виджетов и кнопки перед созданием новых


    // Создание новых виджетов и кнопки
    // ... ваш код создания виджетов и кнопки ...

    QSqlQuery query;
    if (!query.exec("SELECT * FROM " + tableName)) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return;
    }
    QSqlRecord record = query.record();

    // Set the label text to the current table name
    ui->labelTable->setText(tableName); // Assuming labelTable is the name of your QLabel

    // Create delete data button if it doesn't exist
    if (!deleteButton) {
        deleteButton = new QPushButton("УДАЛИТЬ");
        ui->verticalLayout_3->addWidget(deleteButton);

        // Connect the delete button to the deleteData slot
        connect(deleteButton, &QPushButton::clicked, this, &FormTable::deleteData);
    }

    // Update input fields
    QVBoxLayout *layout = new QVBoxLayout();
    QList<QPair<QString, QWidget*>> inputWidgets; // Store input widgets to access them later
    for (int i = 0; i < record.count(); ++i) {
        QString fieldName = record.fieldName(i);
        if (fieldName.startsWith("id"))
            continue; // Skip fields starting with 'id'

        QHBoxLayout *rowLayout = new QHBoxLayout(); // Create a QHBoxLayout for each label-input pair

        QLabel *label = new QLabel(fieldName);
        rowLayout->addWidget(label); // Add label to the QHBoxLayout

        QWidget *widget = nullptr;
        if (fieldName.startsWith("data")) {
            QDateEdit *dateEdit = new QDateEdit();
            widget = dateEdit;
        } else if (fieldName.contains("combox")) {
            QComboBox *comboBox = new QComboBox();
            // Execute SQL query to populate comboBox
            QString queryText;
            if (fieldName == "name_position_combox") {
                queryText = "SELECT DISTINCT name_position FROM Position";
            } else  if (fieldName == "worker_fullname_combox") {
                queryText = "SELECT DISTINCT worker_fullname FROM Workers";
            }
            qDebug() << "Executing query:" << queryText;
            QSqlQuery comboQuery(queryText);
            if (!comboQuery.exec()) {
                qDebug() << "Error executing query:" << comboQuery.lastError().text();
                continue;
            }
            while (comboQuery.next()) {
                QString value = comboQuery.value(0).toString();
                qDebug() << "Value:" << value;
                comboBox->addItem(value);
            }
            widget = comboBox;
        } else {
            QLineEdit *lineEdit = new QLineEdit();
            widget = lineEdit;
        }
        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // Set size policy to fixed

        rowLayout->addWidget(widget); // Add input widget to the QHBoxLayout
        layout->addLayout(rowLayout); // Add the QHBoxLayout to the main QVBoxLayout

        inputWidgets.append(qMakePair(fieldName, widget)); // Store the widget with the field name
    }
    // Добавление фильтрации для таблицы Otdel по полю name_otdel
    if (tableName == "Otdel") {
        QLabel *filterLabel = new QLabel("Фильтрация по отделу:");
        layout->addWidget(filterLabel);

        QLineEdit *filterLineEdit = new QLineEdit();
        layout->addWidget(filterLineEdit);

        QPushButton *filterButton = new QPushButton("Фильтрация");
        layout->addWidget(filterButton);

        QPushButton *clearFilterButton = new QPushButton("Очистка");
        layout->addWidget(clearFilterButton);

        connect(filterButton, &QPushButton::clicked, this, [this, tableName, filterLineEdit]() {
            QString filterValue = filterLineEdit->text();
            QSqlTableModel *model = qobject_cast<QSqlTableModel*>(ui->tableView->model());
            if (model) {
                model->setFilter("name_otdel = '" + filterValue + "'");
            }
        });

        connect(clearFilterButton, &QPushButton::clicked, this, [this, tableName, filterLineEdit]() {
            filterLineEdit->clear();
            QSqlTableModel *model = qobject_cast<QSqlTableModel*>(ui->tableView->model());
            if (model) {
                model->setFilter("");
            }
        });
    }
    // Clear previous input layout and create a new inputWidget
    QWidget *inputWidget = new QWidget();
    inputWidget->setLayout(layout);

    // Remove previous inputWidget from the main input layout
    QLayoutItem *item;
    while ((item = ui->verticalLayout_2->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add the new inputWidget to the main input layout
    ui->verticalLayout_2->addWidget(inputWidget);

    // Add a QPushButton to submit data
    QPushButton *submitButton = new QPushButton("ДОБАВИТЬ");
    ui->verticalLayout_2->addWidget(submitButton);
    // Подключение кнопки update к слоту updateData
    // Удаление старых соединений
    disconnect(ui->pushUpdate, &QPushButton::clicked, nullptr, nullptr);
    disconnect(submitButton, &QPushButton::clicked, nullptr, nullptr);

    // Установка новых соединений
    connect(ui->pushUpdate, &QPushButton::clicked, this, [this, inputWidgets]() {
        this->updateData(inputWidgets);
    });
    connect(submitButton, &QPushButton::clicked, this, [this, inputWidgets, tableName]() {
        this->addData(inputWidgets, tableName);
    });

    // Update table view
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable(tableName);
    model->select();
    ui->tableView->setModel(model); // Assuming you have a QTableView named tableView in your form
}


void FormTable::addData(const QList<QPair<QString, QWidget*>>& inputWidgets, const QString& tableName) {
    if (userStatus_ == 2) {
        QMessageBox::critical(this, "Access Denied", "You don't have permission to delete data.");
        return;
    }
    QStringList fieldNames;
    QStringList fieldValues;

    // Собираем имена полей и их значения из виджетов для ввода данных
    for (const auto& pair : inputWidgets) {
        const QString& fieldName = pair.first;
        const QWidget* widget = pair.second;

        if (const QLineEdit* lineEdit = qobject_cast<const QLineEdit*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(lineEdit->text());
        } else if (const QDateEdit* dateEdit = qobject_cast<const QDateEdit*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(dateEdit->date().toString("yyyy-MM-dd"));
        } else if (const QComboBox* comboBox = qobject_cast<const QComboBox*>(widget)) {
            fieldNames.append(fieldName);
            fieldValues.append(comboBox->currentText());
        }
    }
    QMessageBox::StandardButton confirmAdd = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите добавить новые данные?", QMessageBox::Yes | QMessageBox::No);
    if (confirmAdd != QMessageBox::Yes) {
        return;
    }

    // Проверяем, что есть поля и значения для добавления
    if (fieldNames.isEmpty() || fieldValues.isEmpty()) {
        QMessageBox::critical(this, "Error", "No fields to add.");
        return;
    }

    // Строим текст SQL-запроса
    QString queryText = "INSERT INTO " + tableName + " (" + fieldNames.join(", ") + ") VALUES (";
    QStringList valuePlaceholders;
    for (int i = 0; i < fieldValues.size(); ++i) {
        valuePlaceholders.append("?");
    }
    queryText += valuePlaceholders.join(", ") + ")";

    // Подготавливаем SQL-запрос
    QSqlQuery query;
    query.prepare(queryText);
    for (const QString& value : fieldValues) {
        query.addBindValue(value);
    }

    // Выполняем SQL-запрос
    if (query.exec()) {
        QMessageBox::information(this, "Success", "Data added successfully.");
        updateTableView(tableName); // Обновляем представление таблицы
    } else {
        QMessageBox::critical(this, "Error", "Failed to add data: " + query.lastError().text());
    }
}


void FormTable::tableButtonClicked() {
        QPushButton *button = qobject_cast<QPushButton*>(sender());
        if (button) {
            currentTableName = button->text(); // Установка текущего имени таблицы
            updateTableView(currentTableName); // Обновление представления таблицы
        }
}


    FormTable::~FormTable()
    {
        delete ui;
        // Clean up deleteButton
        delete deleteButton;
    }

