#include "widget.h"
#include "ui_widget.h"
#include <QtCore/QSharedMemory>
#include <QBuffer>
#include "../include/protocol.h"
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    _pWorkerThread = new Worker;
    connect(_pWorkerThread,SIGNAL(return_result(const QString&)),this,SLOT(onReturnResult(const QString&)));
    _pWorkerThread->start();

    //要使用自定义的类型作为信号参数时需要先注册该类型
    qRegisterMetaType<SharedMemoryRoad>("SharedMemoryRoad");
    qRegisterMetaType<SharedMemoryRoad>("SharedMemoryRoad&");
    ui->setupUi(this);
    if(ConnectDatabase())
        ui->listWidget->addItem("Has successfully linked to the database");
}

Widget::~Widget()
{
    _pWorkerThread->requestInterruption();
    _pWorkerThread->wait();
    _pWorkerThread->exit();
    delete ui;
}

bool Widget::ConnectDatabase()
{
    QSqlDatabase memDatabase;
    memDatabase = QSqlDatabase::addDatabase("QSQLITE", "process");
    memDatabase.setDatabaseName("../process.db");
    if (!memDatabase.open())
    {
        QSqlError error = memDatabase.lastError();
        ui->listWidget->addItem(error.text());
        return false;
    }
    else
    {
        return true;
    }
}

void Widget::on_pushButton_clicked()
{
    QSharedMemory sharedMemory("test");
    if (!sharedMemory.attach())
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
    QBuffer buffer;
    QDataStream in(&buffer);

    Persion persion;
    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);
    int nCount = 0;
    //先读取数据包的数量
    in >> nCount;
    //根据数据包的数量来读取内存中的信息
    for(int i = 0;i<nCount;i++)
    {
        in>>persion;
        QString strInfo = QString("Name:%1,Age:%2").arg(persion.strName).arg(persion.nAge);
        ui->listWidget->addItem(strInfo);
    }
    sharedMemory.unlock();

}

void Widget::onReturnResult(const QString &str)
{
    ui->listWidget->addItem(str);
}
