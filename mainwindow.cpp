#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->logsText->setReadOnly(true);
    setWindowTitle("Chip Vision");
    _predict_done = true;

    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectToChip()));
    connect(ui->selectImgButton, SIGNAL(clicked(bool)), this, SLOT(openPicture()));
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(sendPicture()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(setWidgetSettings(int)));
}

MainWindow::~MainWindow() {
    closeSocket(_socketId);
    delete ui;
}

void MainWindow::connectToChip() {
    image_widget = new ImageLabel(this);
    image_widget->setMinimumSize(300, 200);
    _receiver_thread = new QThread;
    _socketId = connectToServer();
    if (_socketId != -1) {
        _worker = new SocketWorker(_socketId);
        _worker->moveToThread(_receiver_thread);

        connect(_receiver_thread, SIGNAL(started()), _worker, SLOT(receivingMessages()));
        connect(_worker, SIGNAL(messageReceived(QString)), this, SLOT(appendToLog(QString)));
        connect(_worker, SIGNAL(finished(QString)), _receiver_thread, SLOT(quit()));
        connect(_worker, SIGNAL(finished(QString)), this, SLOT(showPicture(QString)));

        char message[256];
        if (readMessage(_socketId, message) != -1) {
            ui->stackedWidget->setCurrentIndex(1);
            ui->logsText->appendPlainText("Connection is " + QString(message));
        }
        else {
            ui->logsText->appendPlainText("Server is not available");
        }
        _predict_done = true;
    }
    else {
        QMessageBox::warning(this, "Error connection",  "Error connect to server!");
    }
}

void MainWindow::sendPicture() {
    _predict_done = false;
    char* char_file_name = _file_name.toLocal8Bit().data();
    char response[256] = "Sending of image successful";
    char command[256] = "yolo";
    // send  command yolo
    writeMessage(_socketId, command);
    readMessage(_socketId, response);
    // send image
    int n = sendImage(_socketId, char_file_name);
    if (n != -1) {
        readMessage(_socketId, response);
        ui->logsText->appendPlainText(QString(response));
        ui->runButton->setEnabled(false);
        ui->selectImgButton->setEnabled(false);
    }
    _receiver_thread->start();
}

void MainWindow::openPicture() {
    QString file_name = QFileDialog::getOpenFileName(this, "Image", QDir::homePath(), "Images (*.jpg *.png)");
    if (!file_name.isEmpty()) {
        if (showPicture(file_name)){
            _file_name = file_name;
            ui->runButton->setEnabled(true);
            enabledTasks(true);
        }
    }
}

void MainWindow::appendToLog(QString message) {
    if (message.isEmpty()) {
        ui->logsText->appendPlainText("Error connect to server!");
        errorExit();
    }
    else {
        ui->logsText->appendPlainText(message);
    }
}

bool MainWindow::showPicture(QString file_name) {
    if (file_name.isEmpty()) {
        char filename[256];
        bzero(filename, sizeof(filename));
        getImage(_socketId, filename);
        file_name = QString(filename);
        _predict_done = true;
        ui->runButton->setEnabled(true);
        ui->selectImgButton->setEnabled(true);
    }
    QPixmap img(file_name);
    if (img.size().height() == 0 && img.size().width() == 0) {
        QMessageBox::warning(this, "Open image error", "Wrong image format");
        return false;
    }
    else {
        image_widget->setPixmap(img);
        return true;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (_predict_done) {
        writeMessage(_socketId, "exit");
        closeSocket(_socketId);
        event->accept();
    }
    else {
        QMessageBox::warning(this, "Server", "Server running");
        event->ignore();
    }
}

void MainWindow::setWidgetSettings(int current) {
    if (current == 1) {
        ui->verticalLayoutTasks->addWidget(image_widget);
        QPixmap img("start.png");
        image_widget->setPixmap(img);
        enabledTasks(false);
    }
}

void MainWindow::enabledTasks(bool enabled) {
    ui->runButton->setEnabled(enabled);
}

void MainWindow::errorExit() {
    delete image_widget;
    ui->stackedWidget->setCurrentIndex(0);
}
