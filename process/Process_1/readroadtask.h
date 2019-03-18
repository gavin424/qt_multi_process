/********************************************************************************
 * ReadRoadTask用于读取共享内存的数据
 * ReadRoadTask::_strKey是共享内存的地址
 * ReadRoadTask对象读取_strKey指定的内存中的数据
 * ReadRoadTask继承自QObject和QRunnable类
 * Qt中QThreadPool管理和处理的线程都必须继承自QRunnable类
 * QRunnable是一个接口而且没有继承自QObject，所以QRunnable不具备信号槽的机制
 * 为了使ReadRoadTask可以使用信号槽机制，ReadRoadTask同时继承自QObject和QRunnable类
********************************************************************************/
#ifndef READROADTASK_H
#define READROADTASK_H
#include <QRunnable>
#include "../include/protocol.h"

class ReadRoadTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ReadRoadTask(const QString& strKey,QObject* parent = Q_NULLPTR);
    ~ReadRoadTask();


    virtual void run();
private:
    QString _strKey;

signals:
    void taskfinished(const QString&);
    void return_result(const SharedMemoryRoad&);
};

#endif // READROADTASK_H
