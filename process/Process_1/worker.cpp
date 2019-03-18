#include "worker.h"
#include <QBuffer>
#include <QSharedMemory>
#include <QElapsedTimer>
#include <QThreadPool>
#include "readroadtask.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>
#include <QSqlError>

void Worker::run()
{
    //ͨ�� QThread::requestInterruption()  �ж�
    while(!this->isInterruptionRequested())
    {
        QElapsedTimer timer;
        timer.start();
        QSharedMemory sharedMemory(SHARED_MEMORY_KEY);
        if(!sharedMemory.attach())
        {
            QSharedMemory::SharedMemoryError error = sharedMemory.error();
            return_result("error code:"+QString::number(error));
            msleep(5*1000);continue;
        }
        QBuffer buffer;
        QDataStream in(&buffer);
        QList<SharedMemoryPointer> lsSmp;
        sharedMemory.lock();
        buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
        buffer.open(QBuffer::ReadOnly);
        int nPackageCount = 0;
        in>>nPackageCount;
        for(int i = 0;i<nPackageCount;i++)
        {
            SharedMemoryPointer smp;
            in>>smp;
            smp.nUserTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            lsSmp.append(smp);
        }
        sharedMemory.unlock();
        for(int i = 0;i<lsSmp.size();i++)
        {
            QString strReturnResult = QString("Pointer:%1\nIsReCover:%2\nModify Time:%3\nUsage Time:%4").arg(lsSmp[i].strKey).
                    arg(lsSmp[i].isRecover).arg(lsSmp[i].nModifyTime).arg(lsSmp[i].nUserTime);
            return_result(strReturnResult);
        }
        int nPointerReadFinishedTimes = timer.elapsed();
        return_result("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        return_result(QString("Read %1 data, taking %2 milliseconds").arg(lsSmp.size()).arg(nPointerReadFinishedTimes));
        return_result("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        timer.restart();
        m_listSharedMemoryRoad.clear();
        for(int i = 0;i<lsSmp.size();i++)
        {
            //�̳߳� ��������QThreadPool��QRunnable
            //QThreadPoolִ�к͹�����Ҫִ�еľ���QRunnable����
            //QThreadPool::globalInstance()��ȡȫ���̳߳ض���
            //QThreadPool::start(QRunnable*)��ʼִ��һ���߳�
            //QThreadPool::waitForDone() �ȴ������߳�ִ�����
            //QRunnable::setAutoDelete(bool)�߳�ִ����ɺ��Ƿ��Զ��ͷ�QRunable����
            ReadRoadTask* task = new ReadRoadTask(lsSmp[i].strKey);
            //Qt�п��߳��ź�������ӷ�ʽ��2��
            //1.Qt::BlockingQueuedConnection �������У��൱��MFC��Send Message()
            //2.Qt::QueuedConnection ���������� ���൱��PostMessage()
            //2�����ӷ�ʽ�����ڽ����߳�ִ��
            //Qt::BlockingQueuedConnection ������ͬһ�߳���ʹ�ã�������������
            connect(task,SIGNAL(taskfinished(const QString&)),this,SLOT(onThreadPoolResult(const QString &)),Qt::BlockingQueuedConnection);
            //connect(task,SIGNAL(taskfinished(const QString&)),this,SLOT(onThreadPoolResult(const QString &)),Qt::QueuedConnection);
            connect(task,SIGNAL(return_result(const SharedMemoryRoad&)),this,SLOT(onReturnResult(const SharedMemoryRoad&)),Qt::QueuedConnection);
            QThreadPool::globalInstance()->start(task);
        }
        //QThreadPool::waitForDone()�������߳�ֱ�����е�Task��ִ�����
        //Qt::BlockingQueuedConnectionģʽ��waitForDone()���ؼ���ʾ���е��ڴ涼��ȡ��������������
        QThreadPool::globalInstance()->waitForDone();
        return_result("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        return_result(QString("Read All Memory info, taking %1 milliseconds").arg(timer.elapsed()));
        return_result("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        timer.restart();
        if(InsertDatabase())
        {
            return_result(QString("Has successfully written to the database,taking %1 millisenonds").arg(timer.elapsed()));
        }
        msleep(30*1000);
    }
}

bool Worker::InsertDatabase()
{
    QSqlDatabase db_sqlite3 = QSqlDatabase::database("process");
    QSqlQuery q(db_sqlite3);
    QString strSql;
    strSql = "INSERT INTO t_roadinfo(area,junction,name,direct_num,direct_no,lane_num, lane_no,lane_type) VALUES(?,?,?,?,?,?,?,?)";
    q.prepare(strSql);
    QVariantList lsVarArea;
    QVariantList lsVarJunction;
    QVariantList lsVarName;
    QVariantList lsVarDirectNum;
    QVariantList lsVarDirectNo;
    QVariantList lsVarLaneNum;
    QVariantList lsVarLaneNo;
    QVariantList lsVarLaneTypre;
    for(int i = 0;i<m_listSharedMemoryRoad.size();i++)
    {
        lsVarArea << m_listSharedMemoryRoad[i].nAreaNo;
        lsVarJunction << m_listSharedMemoryRoad[i].nJuncNo;
        lsVarName << m_listSharedMemoryRoad[i].strRoadName;
        lsVarDirectNum << m_listSharedMemoryRoad[i].nDirNum;
        QString strTemp;
        for(int n = 0;n<m_listSharedMemoryRoad[i].nDirNum;n++)
        {
            if(n !=0)
                strTemp += ",";
            strTemp+=QString::number(m_listSharedMemoryRoad[i].nDirNo[n]);
        }
        lsVarDirectNo << strTemp;
        strTemp.clear();
        for(int n = 0;n<m_listSharedMemoryRoad[i].nDirNum;n++)
        {
            if(n !=0)
                strTemp += ",";
            strTemp+=QString::number(m_listSharedMemoryRoad[i].nLaneNum[n]);
        }
        lsVarLaneNum << strTemp;
        strTemp.clear();
        for(int m = 0;m<m_listSharedMemoryRoad[i].nDirNum;m++)
        {
            int nDirNo = m_listSharedMemoryRoad[i].nDirNo[m];
            QString str = "(";
            for(int n = 0;n < m_listSharedMemoryRoad[i].nLaneNum[nDirNo];n++)
            {
                if(n !=0)
                    str += ",";
                str += QString::number(m_listSharedMemoryRoad[i].nLaneNo[nDirNo][n]);
            }
            str += ")";
            strTemp += str;
        }
        lsVarLaneNo << strTemp;
        strTemp.clear();
        for(int m = 0;m<m_listSharedMemoryRoad[i].nDirNum;m++)
        {
            int nDirNo = m_listSharedMemoryRoad[i].nDirNo[m];
            QString str = "(";
            for(int n = 0;n < m_listSharedMemoryRoad[i].nLaneNum[nDirNo];n++)
            {
                if(n !=0)
                    str += ",";
                str += m_listSharedMemoryRoad[i].strLaneType[nDirNo][n];
            }
            str += ")";
            strTemp += str;
        }
        lsVarLaneTypre << strTemp;
    }
    q.addBindValue(lsVarArea);
    q.addBindValue(lsVarJunction);
    q.addBindValue(lsVarName);
    q.addBindValue(lsVarDirectNum);
    q.addBindValue(lsVarDirectNo);
    q.addBindValue(lsVarLaneNum);
    q.addBindValue(lsVarLaneNo);
    q.addBindValue(lsVarLaneTypre);
    db_sqlite3.transaction();
    if (!q.execBatch())
    {
        db_sqlite3.rollback();
        QString strErrorSql = q.lastQuery();
        QSqlError error = q.lastError();
        QString strError = "error infomation:" + error.text() + "\nlast error sql:";
        return_result(strError+strErrorSql);
        return false;
    }
    db_sqlite3.commit();
    return true;
}

void Worker::onThreadPoolResult(const QString &str)
{
    return_result(str);
}

void Worker::onReturnResult(const SharedMemoryRoad &smr)
{
    static int nFrequency = 0;
    nFrequency++;
    QString strResult;
    strResult += QString("Road Name:%1,Area No:%2,Juntion No:%3,Direction Count:%4").
            arg(smr.strRoadName).arg(smr.nAreaNo).arg(smr.nJuncNo).arg(smr.nDirNum);
    strResult += "\n";
    QString strTemp;
    for(int i = 0;i>smr.nDirNum;i++)
    {
        if(i !=0)
            strTemp += ",";
        strTemp+=QString::number(smr.nDirNo[i]);
    }
    strResult += ("Direction No:"+strTemp);
    strResult += "\n";
    strTemp.clear();
    for(int i = 0;i>smr.nDirNum;i++)
    {
        int nDirNo = smr.nDirNo[i];
        if(i !=0)
            strTemp += ",";
        strTemp+=QString::number(smr.nLaneNum[nDirNo]);
    }
    strResult += ("Lane Number:"+strTemp);
    strResult += "\n";
   for(int i = 0;i<smr.nDirNum;i++)
   {
        strTemp.clear();
        int nDirNo = smr.nDirNo[i];
        strTemp = QString("[%1]Direction:").arg(nDirNo);
        for(int n = 0;n<smr.nLaneNum[nDirNo];n++)
        {
            if(n != 0)
                strTemp += ",";
            strTemp += QString("Lane No[%1](%2)").arg(smr.nLaneNo[nDirNo][n]).
                    arg(smr.strLaneType[nDirNo][n]);
        }
        strResult += strTemp;
        if(i != smr.nDirNum-1)
            strResult += "\n";
   }
   return_result(strResult);
//   //ÿһ��·������copy10000��
//   SharedMemoryRoad smrCopy = smr;
//   for(int i = 0;i<5;i++)
//   {
//        smrCopy.nAreaNo = nFrequency;
//        smrCopy.nJuncNo = i;
//        m_listSharedMemoryRoad.append(smrCopy);
//   }

   m_listSharedMemoryRoad.append(smr);
}
