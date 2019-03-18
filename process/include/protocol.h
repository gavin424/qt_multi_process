/********************************************************************************
** protocol.h�ж����˲�ͬ���̼�������ݽ���������Э��
**���������˱�׼���������operator << �� operator >>
** ���ر�׼�����������Ŀ�ǽ��Զ�������ݽṹת����
**�����Ƶ�������
** �ڹ����ڴ�QSharedMemory�н�֧�ֱ�׼�������͵Ķ���������д��ʽ
** ͨ����������������Ϳ��Խ���QDataStream�Զ��������ķ�ʽд�뵽QBuffer
** QBuffer��QIODevice�Ľӿ���Qt�ٷ��İ����ĵ��б���Ϊ��
** ��The QBuffer class provides a QIODevice interface for a QByteArray.��
********************************************************************************/
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QDataStream>
#include <QString>
#include <QDateTime>

//����һ�����Ե����ݽṹ����û���κ�����
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

//SharedMemoryPointer ׼������ָ���Ѵ����Ĺ����ڴ��ַ
//����Ҫ�������ǹ���ǰ�Ѵ����Ĺ����ڴ�
//strKey����ָ�����ڴ�ĵ�ַ,���ȹ̶�8λ
//isRecover���ڱ����ָ��Ĺ����ڴ��Ƿ���Ҫ����
//nModifyTime ��¼�޸�ʱ�䣬ʱ��ʹ��UTC��ʾ��ȷ��ms
//nUserTime ��¼ʹ��ʱ��
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

//·�ڵ���Ϣ
//������
//strRoadName ·������
//nAreaNo �����
//nJuncNo ·�ں�
//nDirNum ��������
//pDirNo ����ţ�����=sizeof(int)*nDirNum �����0��1��2��3
//pLaneNum ���������ϵĳ������� �Է������Ϊ�±�
//pLaneNo ���������ϵĳ����� �Է����Ϊ�±꣬
//pLaneType ���������ϵĳ������ͣ��Է����Ϊ�±꣬
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
    //һ��Hash�㷨��������һ��Ψһ��Hashֵ��ΪQSharedMemory��key
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
