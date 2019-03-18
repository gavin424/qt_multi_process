/*************************************************************************
 * QObject::moveToThread��Qt�����̵߳���һ�ַ�ʽ
 * ��This function is not thread-safe; the current thread must be same as the current thread affinity.
 * In other words, this function can only "push" an object from the current thread to another thread,
 * it cannot "pull" an object from any arbitrary thread to the current thread.��
 * QObject::moveToThread�Ƿ��̰߳�ȫ�ġ�ֻ�ܴӵ�ǰ�߳��Ƶ������̲߳��ܴ�����
 * �����߳��Ƶ���ǰ�߳�
 * QObject::moveToThread�ǹٷ��Ƽ��Ŀ����̵߳ķ�ʽ
 * ʹ�÷�ʽ
 * Worker* worker = new Worker
 * QThread* thread = new QThread
 * worker->moveToThread(thread)
 * thread->start()
 * ��The code inside the Worker's slot would then execute in a separate thread.
 *  However, you are free to connect the Worker's slots to any signal, from any object, in any thread.
 * It is safe to connect signals and slots across different threads, thanks to a mechanism called queued connections.��
 * ����Qt::QueuedConnection�����ӻ��ƿ��԰�ȫ���ڲ�ͬ�߳��н������ݽ���
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
