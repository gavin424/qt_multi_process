#include "readroadtask.h"

#include <QBuffer>
#include <QSharedMemory>
#include <QElapsedTimer>

ReadRoadTask::ReadRoadTask(const QString &strKey, QObject *parent)
    :QObject(parent),_strKey(strKey)
{
    //执行结束后自动销毁
    setAutoDelete(true);
}

void ReadRoadTask::run()
{
    QElapsedTimer timer;
    timer.start();
    QSharedMemory sharedMemory(_strKey);
    if(!sharedMemory.attach())
    {
        QSharedMemory::SharedMemoryError error = sharedMemory.error();
        QString strError = QString("Failed to read [%1] memory,Error code:%2").arg(_strKey).arg(error);
        taskfinished(strError);
        return;
    }
    QBuffer buffer;
    QDataStream in(&buffer);
    SharedMemoryRoad smr;
    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);
    in >> smr;
    sharedMemory.unlock();
    return_result(smr);
    QString strResult = QString("Read memory [%1] successfully,taking %2 milliseconds").arg(_strKey).arg(timer.elapsed());
    taskfinished(strResult);
}

ReadRoadTask::~ReadRoadTask()
{
    //对象执行结束后断开与该对象taskfinished(const QString&)的所有槽
    this->disconnect(SIGNAL(taskfinished(const QString&)));
    this->disconnect(SIGNAL(return_result(const SharedMemoryRoad&)));
}
