#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "worker.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    bool ConnectDatabase();

private slots:
    void on_pushButton_clicked();
    void onReturnResult(const QString& str);

private:
    Ui::Widget *ui;
    Worker* _pWorkerThread;
};

#endif // WIDGET_H
