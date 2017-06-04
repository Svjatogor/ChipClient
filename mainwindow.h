#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "tcp_client.h"
#include "socketworker.h"
#include <QThread>
#include "imagelabel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void connectToChip();
    void sendPicture();
    void openPicture();
    void appendToLog(QString message);
    bool showPicture(QString file_name);
    void setWidgetSettings(int);
    void enabledTasks(bool enabled);
    void errorExit();

private:
    Ui::MainWindow *ui;
    int _socketId;
    QString _file_name;
    SocketWorker* _worker;
    QThread* _receiver_thread;
    void closeEvent(QCloseEvent *event);
    ImageLabel* image_widget;
    bool _predict_done;
};

#endif // MAINWINDOW_H
