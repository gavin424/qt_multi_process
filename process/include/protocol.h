/********************************************************************************
** protocol.h中定义了不同进程间进行数据交互的数据协议
**并且重载了标准流输入输出operator << 和 operator >>
** 重载标准输入输出流的目是将自定义的数据结构转换成
**二进制的数据流
** 在共享内存QSharedMemory中仅支持标准数据类型的二进制流读写方式
** 通过重载输入输出流就可以借助QDataStream以二进制流的方式写入到QBuffer
** QBuffer是QIODevice的接口在Qt官方的帮助文档中表述为：
** “The QBuffer class provides a QIODevice interface for a QByteArray.”
********************************************************************************/
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QDataStream>
#include <QString>
#include <QDateTime>

//这是一个测试的数据结构，并没有任何作用
typedef struct
{
    QString strName;
    int nAge;
} Persion;

inline QDataStream& operator <<(QDataStream& out,const Persion& p)
{
    out << p.strName << p.nAge;
    return out;
}

inline QDataStream& operator >>(QDataStream& in,Persion& p)
{
    in >> p.strName >> p.nAge;
    return in;
}

//SharedMemoryPointer 准备用于指向已创建的共享内存地址
//其主要的作用是管理当前已创建的共享内存
//strKey用于指向共享内存的地址,长度固定8位
//isRecover用于标记所指向的共享内存是否需要回收
//nModifyTime 记录修改时间，时间使用UTC表示精确到ms
//nUserTime 记录使用时间
struct SharedMemoryPointer
{
    QString strKey;
    bool isRecover;
    qint64 nModifyTime;
    qint64 nUserTime;
};

inline QDataStream& operator <<(QDataStream& out,const SharedMemoryPointer& smp)
{
    out << smp.strKey << smp.isRecover<<smp.nModifyTime<<smp.nUserTime;
    return out;
}

inline QDataStream& operator >>(QDataStream& in,SharedMemoryPointer& smp)
{
    in >> smp.strKey >> smp.isRecover>>smp.nModifyTime>>smp.nUserTime;
    return in;
}

//路口的信息
//包括：
//strRoadName 路口名称
//nAreaNo 区域号
//nJuncNo 路口号
//nDirNum 方向数量
//pDirNo 方向号，长度=sizeof(int)*nDirNum 方向号0，1，2，3
//pLaneNum 各个方向上的车道数量 以方向号作为下标
//pLaneNo 各个方向上的车道号 以方向号为下标，
//pLaneType 各个方向上的车道类型，以方向号为下标，
struct SharedMemoryRoad
{
    QString strRoadName;
    int nAreaNo;
    int nJuncNo;
    int nDirNum;
    int nDirNo[4];
    int nLaneNum[4];
    int nLaneNo[4][8];
    QString strLaneType[4][8];
    //一种Hash算法用于生成一个唯一的Hash值作为QSharedMemory的key
    QString Key()
    {
        QString str = strRoadName + QString::number(nAreaNo)+QString::number(nJuncNo)+QString::number(nDirNum);
        uint seed = 131;
        uint hash = 0;
        QByteArray ba = str.toLatin1();
        for (int i = 0; i < ba.size(); ++i)
        {
            hash = hash*seed + (ba[i]);
        }
        return ("smr_"+QString::number(hash & 0x7FFFFFFF));
    }
};

inline QDataStream& operator <<(QDataStream& out,const SharedMemoryRoad& smr)
{
    out << smr.strRoadName << smr.nAreaNo<< smr.nJuncNo << smr.nDirNum;
    for(int i = 0;i<smr.nDirNum;i++)
    {
        out << smr.nDirNo[i];
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        out << smr.nLaneNum[nDirNo];
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        int nLaneNum =  smr.nLaneNum[nDirNo];
        for(int n = 0;n<nLaneNum;n++)
        {
            out << smr.nLaneNo[nDirNo][n];
        }
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        int nLaneNum =  smr.nLaneNum[nDirNo];
        for(int n = 0;n<nLaneNum;n++)
        {
            out << smr.strLaneType[nDirNo][n];
        }
    }
    return out;
}

inline QDataStream& operator >> (QDataStream& in,SharedMemoryRoad& smr)
{
    in >> smr.strRoadName >> smr.nAreaNo >> smr.nJuncNo >> smr.nDirNum;
    for(int i = 0;i<smr.nDirNum;i++)
    {
        in >> smr.nDirNo[i];
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        in >> smr.nLaneNum[nDirNo];
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        int nLaneNum =  smr.nLaneNum[nDirNo];
        for(int n = 0;n<nLaneNum;n++)
        {
            in >> smr.nLaneNo[nDirNo][n];
        }
    }
    for(int i = 0;i<smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        int nLaneNum =  smr.nLaneNum[nDirNo];
        for(int n = 0;n<nLaneNum;n++)
        {
            in >> smr.strLaneType[nDirNo][n];
        }
    }
    return in;
}


#define SHARED_MEMORY_KEY QStringLiteral("shared_memory_manager")

#endif // PROTOCOL_H
