/*************************************************************************
 * QObject::moveToThread是Qt开启线程的另一种方式
 * “This function is not thread-safe; the current thread must be same as the current thread affinity.
 * In other words, this function can only "push" an object from the current thread to another thread,
 * it cannot "pull" an object from any arbitrary thread to the current thread.”
 * QObject::moveToThread是非线程安全的。只能从当前线程移到其他线程不能从其他
 * 任意线程移到当前线程
 * QObject::moveToThread是官方推荐的开启线程的方式
 * 使用方式
 * Worker* worker = new Worker
 * QThread* thread = new QThread
 * worker->moveToThread(thread)
 * thread->start()
 * “The code inside the Worker's slot would then execute in a separate thread.
 *  However, you are free to connect the Worker's slots to any signal, from any object, in any thread.
 * It is safe to connect signals and slots across different threads, thanks to a mechanism called queued connections.”
 * 利用Qt::QueuedConnection的链接机制可以安全的在不同线程中进行数据交互
*************************************************************************/
#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "../include/protocol.h"
#include <QSharedMemory>
#include <QList>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void executetimes(qint64 ms);

public slots:
    void on_dialog_start();

private:
    QSharedMemory m_sharedMemoryManage;
    QList<QSharedMemory*> m_listSharedMemory;
};

#endif // WORKER_H
