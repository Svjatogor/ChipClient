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

    _receiver_thread = new QThread;

    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectToChip()));
    connect(ui->selectImgButton, SIGNAL(clicked(bool)), this, SLOT(openPicture()));
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(sendPicture()));
    connect(ui->radioButtonClassif, SIGNAL(toggled(bool)), this, SLOT(setComboBoxList()));
    connect(ui->radioButtonDetect, SIGNAL(toggled(bool)), this, SLOT(setComboBoxList()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(setWidgetSettings(int)));

    image_widget = new ImageLabel(this);
    image_widget->setMinimumSize(300, 200);
    ui->radioButtonDetect->setChecked(true);
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
    char* conf_type;
    char response[256] = "";
    if (ui->radioButtonDetect->isChecked()) {// detection command
        // send  command yolo
        writeMessage(_socketId, "yolo");
        readMessage(_socketId, response);
        // send image
        int n = sendImage(_socketId, char_file_name);
        if (n != -1) {
            readMessage(_socketId, response);
            ui->logsText->appendPlainText(QString(response));
        }
        // send configuration type
        conf_type = ui->comboBoxModels->currentText().toLocal8Bit().data();
        writeMessage(_socketId, conf_type);
        _receiver_thread->start();
    }
}

void MainWindow::openPicture() {
    _file_name = QFileDialog::getOpenFileName(this, "Image", QDir::homePath(), "Images (*.jpg *.png)");
    if (!_file_name.isEmpty()) {
        ui->runButton->setEnabled(true);
        showPicture(_file_name);
        enabledTasks(true);
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
    image_widget->setPixmap(img);
}

void MainWindow::setComboBoxList() {
    ui->comboBoxModels->clear();
    QStringList list_items;
    if (ui->radioButtonClassif->isChecked()) {
        list_items << "AlexNet" << "Darknet Reference" << "VGG-16" << "Darknet19";
    }
    else {
        list_items << "YOLOv2" << "YOLOv2 544x544" << "Tiny YOLO";
    }
    ui->comboBoxModels->insertItems(0, list_items);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeMessage(_socketId, "exit");
    closeSocket(_socketId);
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
    ui->groupBoxTasks->setEnabled(enabled);
    ui->comboBoxModels->setEnabled(enabled);
    ui->runButton->setEnabled(enabled);
}
