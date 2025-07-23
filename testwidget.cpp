#include "testwidget.h"
#include "ui_testwidget.h"
#include "communicationport.h"
#include <QTimer>
#include <QDebug>
#include <QBuffer>
#include <QByteArray>
#include <QFile>
TestWidget::TestWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestWidget)
{
    ui->setupUi(this);
    m_communiPtr = new CommunicationPort(UDP_PORT,this);
    connect(m_communiPtr,&CommunicationPort::signalReceived,this,&TestWidget::onReceiveData);
    connect(m_communiPtr,&CommunicationPort::signalOpened,this,&TestWidget::onOpened);
    connect(m_communiPtr,&CommunicationPort::signalError,this,&TestWidget::onError);
    connect(m_communiPtr,&CommunicationPort::signalClosed,this,&TestWidget::onClosed);
}

TestWidget::~TestWidget()
{
    delete ui;
}

void TestWidget::on_openBtn_clicked()
{
    //m_communiPtr->open("/dev/ttymxc0",115200);
    //m_communiPtr->open("/dev/ttyUSB0",115200);
    m_communiPtr->open("192.168.2.100",9999);
}

void TestWidget::on_bindBtn_clicked()
{
    //若使用网络作为接收端口 需要绑定端口
    //此处暂时只考虑作为发送端
    m_communiPtr->bindPort(9999);
}

void TestWidget::on_sendBtn_clicked()
{
    //单帧数据发送测试
   /* m_timer = new QTimer(this);
    connect(m_timer,&QTimer::timeout,[this](){
        QByteArray data("666");
        m_communiPtr->sendDatas(CMD_SHORTDATA,data);
    });
    m_timer->start(3000);*/

    //分包传输测试
    // 1. 检查资源是否存在
    if (!QFile::exists(":/images/test.png")) {
        qDebug() << "Error: Resource not found!";
        return;
    }
    QPixmap pixmap(":/images/test.png"); // 支持资源路径或本地路径
    if (pixmap.isNull()) {
        qDebug() << "Error: Failed to load image.";
        return;
    }
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly); // 必须为WriteOnly模式
    bool success = pixmap.save(&buffer, "PNG");
    if (!success) {
        qDebug() << "Error: Failed to save image to buffer.";
        return;
    }
    buffer.close();
    qDebug() << "byteArray size() " << byteArray.size() << endl;
    m_communiPtr->sendDatas(CMD_PICTURETRANSMIT,byteArray);

}


void TestWidget::on_stopBtn_clicked()
{
    if(m_timer)
    {
        m_timer->stop();
    }
}

void TestWidget::onReceiveData(REPORT_COMMAND cmd, const QByteArray &data)
{
    if(cmd == CMD_PICTURETRANSMIT)
    {
        ui->label->setPixmap(QPixmap(QString::fromUtf8(data)));
    }else if(cmd == CMD_SHORTDATA){
        ui->textBrowser->append(QString::number(cmd));
        ui->textBrowser->append(data);
    }



}

void TestWidget::onError(QString str)
{
    qDebug() << "error" << str << endl;
}

void TestWidget::onOpened()
{

}

void TestWidget::onClosed()
{

}
