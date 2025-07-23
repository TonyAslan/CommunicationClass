#ifndef TESTWIDGET_H
#define TESTWIDGET_H
#include "command.h"
#include <QWidget>
class CommunicationPort;
class QTimer;
namespace Ui {
class TestWidget;
}

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestWidget(QWidget *parent = nullptr);
    ~TestWidget();

private slots:
    void on_openBtn_clicked();

    void on_bindBtn_clicked();

    void on_sendBtn_clicked();

    void on_stopBtn_clicked();

private slots:
    void onReceiveData(REPORT_COMMAND cmd, const QByteArray& data);
    void onError(QString);
    void onOpened();
    void onClosed();


private:
    Ui::TestWidget *ui;

    CommunicationPort* m_communiPtr;

    QTimer* m_timer;

};

#endif // TESTWIDGET_H
