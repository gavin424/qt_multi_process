/**************************************************************************************
 * Worker继承自QThread
 * QThread是一个独立与平台线程管理类
 * QThread继承自QObject，可以使用信号槽机制与其他线程通信
 * 在Qt开启一个线程一般有两种方式：
 * 1.子类化QThread并且override QThread::run()函数，本类即使用了该方法
 * 2.使用QObject::moveToThread() 在Process_0工程中使用的方法
**************************************************************************************/
#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include "../include/protocol.h"

class Worker : public QThread
{
    Q_OBJECT

protected:
    void run() Q_DECL_OVERRIDE;
    bool InsertDatabase();

signals:
    void return_result(const QString&);
public slots:
    void onThreadPoolResult(const QString& str);
    void onReturnResult(const SharedMemoryRoad& smr);
private:
    QList<SharedMemoryRoad> m_listSharedMemoryRoad;

};

#endif // WORKER_H
