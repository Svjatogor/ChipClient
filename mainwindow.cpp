#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    setWindowTitle("Chip Vision");

    _receiver_thread = new QThread;

    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectToChip()));
    connect(ui->selectImgButton, SIGNAL(clicked(bool)), this, SLOT(openPicture()));
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(sendPicture()));
}

MainWindow::~MainWindow() {
    closeSocket(_socketId);
    delete ui;
}

void MainWindow::connectToChip() {
    _socketId = connectToServer();
    if (_socketId != -1) {
        _worker = new SocketWorker(_socketId);
        _worker->moveToThread(_receiver_thread);
        connect(_receiver_thread, SIGNAL(started()), _worker, SLOT(receivingMessages()));
        connect(_worker, SIGNAL(messageReceived(QString)), this, SLOT(appendToLog(QString)));
        connect(_worker, SIGNAL(finished(QString)), _receiver_thread, SLOT(quit()));
        connect(_worker, SIGNAL(finished(QString)), this, SLOT(showPicture(QString)));

        char message[256];
        ui->stackedWidget->setCurrentIndex(1);
        readMessage(_socketId, message);
        ui->logsText->appendPlainText("Connection is " + QString(message));
    }
    else {
        QMessageBox::warning(this, "Error connection",  "Error connect to server");
    }
}

void MainWindow::sendPicture() {
    char* char_file_name = _file_name.toLocal8Bit().data();
    // send image
    writeMessage(_socketId, "run");
    int n = sendImage(_socketId, char_file_name);
    if (n != -1) {
        ui->logsText->appendPlainText("\nSending of image successful");
    }
    _receiver_thread->start();
}

void MainWindow::openPicture() {
    _file_name = QFileDialog::getOpenFileName(this, "Image", QDir::homePath(), "Images (*.jpg *.png)");
    if (!_file_name.isEmpty()) {
        ui->runButton->setEnabled(true);
        showPicture(_file_name);
    }
}

void MainWindow::appendToLog(QString message) {
    ui->logsText->appendPlainText(message);
}

void MainWindow::showPicture(QString file_name) {
    if (file_name.isEmpty()) {
        char filename[256];
        bzero(filename, sizeof(filename));
        get_image(_socketId, filename);
        file_name = QString(filename);
    }
    QPixmap img(file_name);
    int w = ui->labelImage->width();
    int h = ui->labelImage->height();
    ui->labelImage->setPixmap(img.scaled(w, h, Qt::KeepAspectRatio));
}
