/********************************************************************************
 * ReadRoadTask���ڶ�ȡ�����ڴ������
 * ReadRoadTask::_strKey�ǹ����ڴ�ĵ�ַ
 * ReadRoadTask�����ȡ_strKeyָ�����ڴ��е�����
 * ReadRoadTask�̳���QObject��QRunnable��
 * Qt��QThreadPool����ʹ�����̶߳�����̳���QRunnable��
 * QRunnable��һ���ӿڶ���û�м̳���QObject������QRunnable���߱��źŲ۵Ļ���
 * Ϊ��ʹReadRoadTask����ʹ���źŲۻ��ƣ�ReadRoadTaskͬʱ�̳���QObject��QRunnable��
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
