#include "worker.h"

#include <QBuffer>
#include <QElapsedTimer>

//随机生成路口名称
const QString RoadName[20] = {QStringLiteral("北京东路?南空北大门"),QStringLiteral("北京东路?军区北大门"),QStringLiteral("珠江路?西大影壁"),QStringLiteral("龙蟠路?新庄"),
                              QStringLiteral("龙蟠路?新世纪酒店"),QStringLiteral("龙蟠路?湖景花园"),QStringLiteral("红山路?中北巴士"),QStringLiteral("宁栖路?锁东路"),
                             QStringLiteral("宁栖路?南师大"),QStringLiteral("宁栖路?樱驼花园"),QStringLiteral("宁栖路?东方城"),QStringLiteral("宁杭公路?理工大二号门"),
                             QStringLiteral("宁杭公路?49路站"),QStringLiteral("宁杭公路?理工大三号门"),QStringLiteral("中山门大街┷灵谷寺路"),QStringLiteral("长巷?银城东苑"),
                             QStringLiteral("经五路?新伊汽配城"),QStringLiteral("长江路?洪武北路"),QStringLiteral("长江路?网巾市"),QStringLiteral("中山路?长江路")};

const QString RoadType[7] = {QStringLiteral("左转"),QStringLiteral("右转"),QStringLiteral("直行"),QStringLiteral("左右"),QStringLiteral("左直"),
                            QStringLiteral("右直"),QStringLiteral("左直右")};

Worker::Worker(QObject *parent) : QObject(parent),m_sharedMemoryManage(SHARED_MEMORY_KEY)
{

}

void Worker::on_dialog_start()
{
    QElapsedTimer timer;
    timer.start();
    SharedMemoryRoad lsSmr[20];
    SharedMemoryPointer lsSmp[20];
    for(int i = 0;i<20;i++)
    {
        lsSmr[i].strRoadName = RoadName[i];
        lsSmr[i].nAreaNo = (i%2) == 0 ? 0:1;
        lsSmr[i].nJuncNo = i;
        lsSmr[i].nDirNum = 4;
        for(int n = 0;n<lsSmr[i].nDirNum;n++)
        {
            lsSmr[i].nDirNo[n] = n;
            int nDirNo = lsSmr[i].nDirNo[n];
            lsSmr[i].nLaneNum[nDirNo] = 4;
            for(int m = 0;m<lsSmr[i].nLaneNum[nDirNo];m++)
            {
                lsSmr[i].nLaneNo[nDirNo][m] = m;
                qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch()%qrand());
                int idx = qrand()%7;
                lsSmr[i].strLaneType[nDirNo][m] = RoadType[idx];
            }
        }
    }
    for(int i = 0;i<20;i++)
    {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << lsSmr[i];
        QString strKey = lsSmr[i].Key();
        //这里使用局部QSharedMemory对象在其他进程读取时会返回QSharedMemory::NotFound错误
        //使用局部变量在该函数执行结束后释放局部的QSharedMemory对象
        QSharedMemory *sharedMemory = new QSharedMemory(strKey);
        if(!sharedMemory->isAttached())
        {
            sharedMemory->detach();
        }
        int size = buffer.size();
        if (!sharedMemory->create(size))
        {
            executetimes(0xFFFFFF);
            continue;
        }
        sharedMemory->lock();
        memcpy(sharedMemory->data(),buffer.data().data(),qMin(sharedMemory->size(),size));
        sharedMemory->unlock();
        lsSmp[i].strKey = strKey;
        lsSmp[i].isRecover = false;
        QDateTime t = QDateTime::currentDateTime();
        lsSmp[i].nModifyTime = t.toMSecsSinceEpoch();
        lsSmp[i].nUserTime = t.toMSecsSinceEpoch();
        m_listSharedMemory.append(sharedMemory);
    }
    QBuffer bufferPointer;
    bufferPointer.open(QBuffer::ReadWrite);
    QDataStream outPointer(&bufferPointer);
    outPointer << 20;
    for(int i = 0;i<20;i++)
    {
        outPointer << lsSmp[i];
    }
    //当前进程是否已经关联共享内存
    if(m_sharedMemoryManage.isAttached())
    {
        //如果已经关联则从当前进程分离
        m_sharedMemoryManage.detach();
        //调用QSharedMemory::detach()系统会自动释放该部分内存
    }
    int sizePointer = bufferPointer.size();
    if (!m_sharedMemoryManage.create(sizePointer))
    {
        return;
    }
    m_sharedMemoryManage.lock();
    memcpy(m_sharedMemoryManage.data(),bufferPointer.data().data(),qMin(m_sharedMemoryManage.size(),sizePointer));
    m_sharedMemoryManage.unlock();
    qint64 ms = timer.elapsed();
    executetimes(ms);
}
