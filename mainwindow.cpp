#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);
    // window settings
    setWindowTitle("Chip Vision");
    ui->stackedWidget->setCurrentIndex(0);
    ui->logsText->setReadOnly(true);
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

/**
 * Connecting to the server
 * @brief MainWindow::connectToChip
 */
void MainWindow::connectToChip() {
    // get client socket
    _socketId = connectToServer();
    // init image widget
    image_widget = new ImageLabel(this);
    image_widget->setMinimumSize(300, 200);
    // thread for messaging with the server
    _receiver_thread = new QThread;
    if (_socketId != -1) {
        // start a thread for messaging
        _worker = new SocketWorker(_socketId);
        _worker->moveToThread(_receiver_thread);

        connect(_receiver_thread, SIGNAL(started()), _worker, SLOT(receivingMessages()));
        connect(_worker, SIGNAL(messageReceived(QString)), this, SLOT(appendToLog(QString)));
        connect(_worker, SIGNAL(finished(QString)), _receiver_thread, SLOT(quit()));
        connect(_worker, SIGNAL(finished(QString)), this, SLOT(showPicture(QString)));

        // Receiving a server response
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

/**
 * Sends the image to the server, starts the detection
 * @brief MainWindow::sendPicture
 */
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
        // waiting for a response to receive an image
        readMessage(_socketId, response);
        ui->logsText->appendPlainText(QString(response));
        // enabled buttons
        ui->runButton->setEnabled(false);
        ui->selectImgButton->setEnabled(false);
    }
    _receiver_thread->start();
}

/**
 * Open image file
 * @brief MainWindow::openPicture
 */
void MainWindow::openPicture() {
    QString file_name = QFileDialog::getOpenFileName(this, "Image", QDir::homePath(), "Images (*.jpg *.png)");
    if (!file_name.isEmpty()) {
        // set image to widget
        if (showPicture(file_name)) {
            _file_name = file_name;
            ui->runButton->setEnabled(true);
        }
    }
}

/** Append text to log widget
 * @brief MainWindow::appendToLog
 * @param message
 */
void MainWindow::appendToLog(QString message) {
    if (message.isEmpty()) {
        // if message is empty server is closed
        ui->logsText->appendPlainText("Error connect to server!");
        errorExit();
    }
    else {
        ui->logsText->appendPlainText(message);
    }
}

/**
 * Set image to widget
 * @brief MainWindow::showPicture
 * @param file_name
 * @return
 */
bool MainWindow::showPicture(QString file_name) {
    if (file_name.isEmpty()) {
        // get image from server
        char filename[256];
        bzero(filename, sizeof(filename));
        getImage(_socketId, filename);
        file_name = QString(filename);
        // predict finished, enabled buttons
        _predict_done = true;
        ui->runButton->setEnabled(true);
        ui->selectImgButton->setEnabled(true);
    }
    // open image
    QPixmap img(file_name);
    // if the image is correct, set to widget
    if (img.size().height() == 0 && img.size().width() == 0) {
        QMessageBox::warning(this, "Open image error", "Wrong image format");
        return false;
    }
    else {
        image_widget->setPixmap(img);
        return true;
    }
}

/**
 * Closing the client
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    // if the server has finished working, send a request to quit
    if (_predict_done) {
        writeMessage(_socketId, "exit");
        // close socket
        closeSocket(_socketId);
        event->accept();
    }
    else {
        QMessageBox::warning(this, "Server", "Server running");
        event->ignore();
    }
}

/**
 * Set settings to base window
 * @brief MainWindow::setWidgetSettings
 * @param current
 */
void MainWindow::setWidgetSettings(int current) {
    if (current == 1) {
        // set image widget
        ui->verticalLayoutTasks->addWidget(image_widget);
        // add to widget start image
        QPixmap img("start.png");
        image_widget->setPixmap(img);
        ui->runButton->setEnabled(false);
    }
}

/**
 * Exiting current window
 * @brief MainWindow::errorExit
 */
void MainWindow::errorExit() {
    delete image_widget;
    ui->stackedWidget->setCurrentIndex(0);
}
