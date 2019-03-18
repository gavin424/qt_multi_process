/**************************************************************************************
 * Worker�̳���QThread
 * QThread��һ��������ƽ̨�̹߳�����
 * QThread�̳���QObject������ʹ���źŲۻ����������߳�ͨ��
 * ��Qt����һ���߳�һ�������ַ�ʽ��
 * 1.���໯QThread����override QThread::run()���������༴ʹ���˸÷���
 * 2.ʹ��QObject::moveToThread() ��Process_0������ʹ�õķ���
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
