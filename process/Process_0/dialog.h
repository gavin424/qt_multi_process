#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSharedMemory>
#include <QDataStream>
#include <QtCore\QProcess>
#include <QTimer>
#include "worker.h"
#include <QThread>

#define TIME_OUT (5*1000)


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
signals:
    void start();
private slots:
    void on_pushButton_clicked();
    void on_btnProcess1_clicked();
    void on_processState_timeout();

    void on_pushButton_2_clicked();
    void onExecutetimes(qint64 ms);

private:
    Ui::Dialog *ui;
    QSharedMemory sharedMemory;
    QProcess* _pProcess;
    int _nTimeId;
    QTimer* _pTimerProcessState;
    Worker* _worker;
    QThread* _thread;
};

#endif // DIALOG_H
