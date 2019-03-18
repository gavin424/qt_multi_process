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
    //由于QShareMemory向系统请求分配内存时，系统会自动对齐
    //举个栗子：在写入进程中写入2个Persion数据包实际大小位30字节
    //但是在读取进程 QSharedMemory的实际大小位4096字节(平台：64位Windows 10)
    //该大小可能与硬件或者系统平台有关系
    //QDataStream::atEnd()并不能表示实际的内存结束位置
    //故
    //为了解决该问题在流的第一段写入数据包的数量
    //读取时根据数据包的数量来读取
    out << 2;
    Persion p;
    p.strName = QStringLiteral("张三");
    p.nAge = 29;
    out << p;
    p.strName = QStringLiteral("李四");
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
    //QProcess 似乎不支持相对路径，只有使用绝对路径才可以打开进程。相对路径会抛出QProcess::FailedToStart错误
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
