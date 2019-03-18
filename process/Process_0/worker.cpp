#include "worker.h"

#include <QBuffer>
#include <QElapsedTimer>

//�������·������
const QString RoadName[20] = {QStringLiteral("������·?�Ͽձ�����"),QStringLiteral("������·?����������"),QStringLiteral("�齭·?����Ӱ��"),QStringLiteral("���·?��ׯ"),
                              QStringLiteral("���·?�����;Ƶ�"),QStringLiteral("���·?������԰"),QStringLiteral("��ɽ·?�б���ʿ"),QStringLiteral("����·?����·"),
                             QStringLiteral("����·?��ʦ��"),QStringLiteral("����·?ӣ�ջ�԰"),QStringLiteral("����·?������"),QStringLiteral("������·?���������"),
                             QStringLiteral("������·?49·վ"),QStringLiteral("������·?����������"),QStringLiteral("��ɽ�Ŵ�֩������·"),QStringLiteral("����?���Ƕ�Է"),
                             QStringLiteral("����·?���������"),QStringLiteral("����·?���䱱·"),QStringLiteral("����·?������"),QStringLiteral("��ɽ·?����·")};

const QString RoadType[7] = {QStringLiteral("��ת"),QStringLiteral("��ת"),QStringLiteral("ֱ��"),QStringLiteral("����"),QStringLiteral("��ֱ"),
                            QStringLiteral("��ֱ"),QStringLiteral("��ֱ��")};

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
        //����ʹ�þֲ�QSharedMemory�������������̶�ȡʱ�᷵��QSharedMemory::NotFound����
        //ʹ�þֲ������ڸú���ִ�н������ͷžֲ���QSharedMemory����
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
    //��ǰ�����Ƿ��Ѿ����������ڴ�
    if(m_sharedMemoryManage.isAttached())
    {
        //����Ѿ�������ӵ�ǰ���̷���
        m_sharedMemoryManage.detach();
        //����QSharedMemory::detach()ϵͳ���Զ��ͷŸò����ڴ�
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
