#include "dialog.h"
#include "ui_dialog.h"
#include <QBuffer>
#include "../include/protocol.h"




Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),sharedMemory("test")
{

    setObjectName("dialog");
    _pProcess = new QProcess(this);
    _pTimerProcessState = new QTimer(this);
    _pTimerProcessState->setObjectName("processState");
    //connect(_pTimerProcessState, SIGNAL(timeout()), this, SLOT(on_processState_timeOut()));
    _pTimerProcessState->start(TIME_OUT);

    _worker = new Worker();
    _worker->setObjectName("worker");
    _thread = new QThread;
    _worker->moveToThread(_thread);
    _thread->start();
    connect(this,SIGNAL(start()),_worker,SLOT(on_dialog_start()));
    connect(_worker,SIGNAL(executetimes(qint64)),this,SLOT(onExecutetimes(qint64)));

    ui->setupUi(this);
}

Dialog::~Dialog()
{
    if(_thread)
    {
        _thread->quit();
        _thread->wait();
        _thread->deleteLater();
    }
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    //����QShareMemory��ϵͳ��������ڴ�ʱ��ϵͳ���Զ�����
    //�ٸ����ӣ���д�������д��2��Persion���ݰ�ʵ�ʴ�Сλ30�ֽ�
    //�����ڶ�ȡ���� QSharedMemory��ʵ�ʴ�Сλ4096�ֽ�(ƽ̨��64λWindows 10)
    //�ô�С������Ӳ������ϵͳƽ̨�й�ϵ
    //QDataStream::atEnd()�����ܱ�ʾʵ�ʵ��ڴ����λ��
    //��
    //Ϊ�˽�������������ĵ�һ��д�����ݰ�������
    //��ȡʱ�������ݰ�����������ȡ
    out << 2;
    Persion p;
    p.strName = QStringLiteral("����");
    p.nAge = 29;
    out << p;
    p.strName = QStringLiteral("����");
    p.nAge = 30;
    out << p;
    if(sharedMemory.isAttached())
        sharedMemory.detach();

    int size = buffer.size();
	if (!sharedMemory.create(size))
	{
		QSharedMemory::SharedMemoryError error =  sharedMemory.error();
        switch (error)
        {
        case QSharedMemory::NoError:
            ui->listWidget->addItem("No error occurred.");
            break;
        case QSharedMemory::PermissionDenied:
            ui->listWidget->addItem("The operation failed because the caller didn't have the required permissions.");
            break;
        case QSharedMemory::InvalidSize:
            ui->listWidget->addItem("A create operation failed because the requested size was invalid.");
            break;
        case QSharedMemory::KeyError:
            ui->listWidget->addItem("The operation failed because of an invalid key.");
            break;
        case QSharedMemory::AlreadyExists:
            ui->listWidget->addItem("A create() operation failed because a shared memory segment with the specified key already existed.");
            break;
        case QSharedMemory::NotFound:
            ui->listWidget->addItem("An attach() failed because a shared memory segment with the specified key could not be found.");
            break;
        case QSharedMemory::LockError:
            ui->listWidget->addItem("The attempt to lock() the shared memory segment failed because create() or attach() failed and returned false,\n"
                                    " or because a system error occurred in QSystemSemaphore::acquire().");
            break;
        case QSharedMemory::OutOfResources:
            ui->listWidget->addItem("A create() operation failed because there was not enough memory available to fill the request.");
            break;
        case QSharedMemory::UnknownError:
            ui->listWidget->addItem("Something else happened and it was bad.");
            break;
        default:
            break;
        }
		return;
	}
    sharedMemory.lock();
    memcpy(sharedMemory.data(),buffer.data().data(),qMin(sharedMemory.size(),size));
    sharedMemory.unlock();
}

void Dialog::on_btnProcess1_clicked()
{
    //QProcess �ƺ���֧�����·����ֻ��ʹ�þ���·���ſ��Դ򿪽��̡����·�����׳�QProcess::FailedToStart����
    QString program = "D:\\svn\\process\\build-Process_1-Desktop_Qt_5_8_0_MSVC2015_32bit-Debug\\debug\\Process_1.exe";
	QStringList arguments;
    _pProcess->start(program, arguments);
    QProcess::ProcessError error = _pProcess->error();
	switch (error)
	{
	case QProcess::FailedToStart:
        ui->listWidget->addItem("The process failed to start. \nEither the invoked program is missing, or you may have insufficient permissions to invoke the program");
		break;
	case QProcess::Crashed:
        ui->listWidget->addItem("The process crashed some time after starting successfully.");
		break;
	case QProcess::Timedout:
        ui->listWidget->addItem("The last waitFor...() function timed out. \nThe state of QProcess is unchanged,and you can try calling waitFor...() again.");
		break;
	case QProcess::ReadError:
        ui->listWidget->addItem("An error occurred when attempting to write to the process. \nFor example,the process may not be running, or it may have closed its input channel.");
		break;
	case QProcess::WriteError:
        ui->listWidget->addItem("An error occurred when attempting to read from the process. \nFor example, the process may not be running.");
		break;
	case QProcess::UnknownError:
        ui->listWidget->addItem("An unknown error occurred. \nThis is the default return value of error().");
		break;
	default:
		break;
    }
}

void Dialog::on_processState_timeout()
{
    QProcess::ProcessState processState = _pProcess->state();
    switch (processState)
    {
    case QProcess::NotRunning:
        ui->listWidget->addItem("The process [process_1] is not running.");
        break;
    case QProcess::Starting:
        ui->listWidget->addItem("The process [process_1] is starting, but the program has not yet been invoked.");
        break;
    case QProcess::Running:
        ui->listWidget->addItem("The process [process_1] is running and is ready for reading and writing.");
        break;
    default:
        break;
    }
}

void Dialog::on_pushButton_2_clicked()
{
    start();
}

void Dialog::onExecutetimes(qint64 ms)
{
    QString strInfo = QString("shared memory written successfully.time consuming %1 ms ").arg(ms);
    ui->listWidget->addItem(strInfo);
}
