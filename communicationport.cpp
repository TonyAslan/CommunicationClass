#include "communicationport.h"
#include "serialdataport.h"
#include "udpdataport.h"
#include "protocolhandler.h"
#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <QImage>

const int MAX_PACKET_SIZE = 1024; // 每个包最大 1Kb
CommunicationPort::CommunicationPort(PortType portType,QObject *parent):
    QObject(parent),
    m_thread(nullptr),
    m_udpDataPort(nullptr),
    m_serialDataPort(nullptr),
    m_protocolHandler(nullptr)
{
    //初始化端口
    switch(portType)
    {
    case PortType::NULL_PORT:
        break;
    case PortType::UDP_PORT:
        m_thread = new QThread;
        m_udpDataPort = new UdpDataPort();
        //向网口操作
        connect(this, SIGNAL(signalOpen(QString, int)), m_udpDataPort, SLOT(onOpen(QString, int)));
        connect(this, SIGNAL(signalWrite(const QByteArray&)), m_udpDataPort, SLOT(onWrite(const QByteArray&)));
        connect(this, SIGNAL(signalClose()), m_udpDataPort, SLOT(onClose()));
        connect(this, SIGNAL(signalBindPort(int)), m_udpDataPort, SLOT(onBindPort(int)));
        //接收网口信号
        connect(m_udpDataPort, SIGNAL(signalReceived(const QByteArray&)), this, SLOT(onReceiveDatas(const QByteArray&)));//发送接收数据
        connect(m_udpDataPort, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
        connect(m_udpDataPort, SIGNAL(signalConnected()), this, SIGNAL(signalOpened()));
        connect(m_udpDataPort, SIGNAL(signalDisconnected()), this, SIGNAL(signalClosed()));
        //响应退出信号
        connect(this, SIGNAL(signalQuiting()), m_udpDataPort, SLOT(deleteLater()));
        connect(m_udpDataPort, SIGNAL(destroyed(QObject*)), m_thread, SLOT(quit()));
        connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
        //响应子线程启动信号
        connect(m_thread, SIGNAL(started()), m_udpDataPort, SLOT(onInit()));
        m_udpDataPort->moveToThread(m_thread);
        m_thread->start();
        break;

    case PortType::SERIAL_PORT:
        m_thread = new QThread;
        m_serialDataPort = new SerialDataPort();
        //向串口操作
        //打开
        connect(this, SIGNAL(signalOpen(QString, int)), m_serialDataPort, SLOT(onOpen(QString, int)));
        //写入
        connect(this, SIGNAL(signalWrite(const QByteArray&)), m_serialDataPort, SLOT(onWrite(const QByteArray&)));
        //关闭
        connect(this, SIGNAL(signalClose()), m_serialDataPort, SLOT(onClose()));
        //接收串口信号
        //接收
        connect(m_serialDataPort, SIGNAL(signalReceived(const QByteArray&)), this, SLOT(onReceiveDatas(const QByteArray&)));//发送接收数据
        //错误
        connect(m_serialDataPort, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
        //连接
        connect(m_serialDataPort, SIGNAL(signalConnected()), this, SIGNAL(signalOpened()));
        //关闭
        connect(m_serialDataPort, SIGNAL(signalDisconnected()), this, SIGNAL(signalClosed()));
        //响应退出信号
        //相应信号 串口删除
        connect(this, SIGNAL(signalQuiting()), m_serialDataPort, SLOT(deleteLater()));
        //串口删除 线程退出
        connect(m_serialDataPort, SIGNAL(destroyed(QObject*)), m_thread, SLOT(quit()));
        //线程退出 然后删除
        connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
        //响应子线程启动信号
        //线程开始 串口初始化
        connect(m_thread, SIGNAL(started()), m_serialDataPort, SLOT(onInit()));
        //移入线程执行
        m_serialDataPort->moveToThread(m_thread);
        m_thread->start();
        break;
    }
    m_portType = portType;
    //初始化协议处理器
    m_protocolHandler = new ProtocolHandler(this);
    //转发完成后处理完整的多包数据
    connect(m_protocolHandler, &ProtocolHandler::signalPacketTransmissionComplete,
            this, &CommunicationPort::onDealCompletePacketData);
    //协议帧校验和错误
    connect(m_protocolHandler, &ProtocolHandler::signalChecksumError,
            this, &CommunicationPort::onCheckError);

}

CommunicationPort::~CommunicationPort()
{
    emit signalQuiting();
}

void CommunicationPort::sendDatas(REPORT_COMMAND cmd, const QByteArray &data)
{
    //分类 有的需要分包传输 有的不需要
    if(cmd == CMD_SHORTDATA)
    {
        //数据比较短的指令 一帧发完
        // 使用 ProtocolHandler 生成帧
        QByteArray frame = m_protocolHandler->createFrame(cmd, data);
        //发送帧
        qDebug() << "sendData : " << frame << "dataLength : " << frame.size() <<endl;
        write(frame);
    }else if(cmd == CMD_PICTURETRANSMIT){
        //图片文件比较大 需要分包传输
        //初始化分包传输
        m_protocolHandler->initPacketTransmission(data,MAX_PACKET_SIZE,cmd);
        QByteArray packet;
        while (m_protocolHandler->getNextPacket(cmd,packet)) {
            // 创建协议帧并发送
            QByteArray frame = m_protocolHandler->createFrame(cmd, packet);
            //通信端口发送帧
            write(frame);
            qDebug() << "total = " << data.size() << " current = " << m_protocolHandler->getCurrentOffset(cmd) << endl;
        }
        //发完发结束标志
        write(m_protocolHandler->createFrame(cmd, QByteArray("#END#")));
    }
}

void CommunicationPort::onReceiveDatas(const QByteArray &rawData)
{
    m_receiveBuffer.append(rawData);
    // 协议处理器解析数据
    QByteArray payload;
    while (m_protocolHandler->validateFrame(m_receiveBuffer, payload)) {
        handleDecodedPayload(static_cast<REPORT_COMMAND>(rawData[2]),payload); // 处理有效载荷
        m_receiveBuffer.remove(0, payload.size() + 8); // 移除已处理数据
    }
}

void CommunicationPort::handleDecodedPayload(REPORT_COMMAND cmd, const QByteArray &payload)
{
    //同样根据指令类别来处理收到的数据
    if(cmd == CMD_PICTURETRANSMIT)
    {
        if(m_protocolHandler->processReceivedPacket(cmd, payload))
        {
            qDebug() << "multipack data receive finish!" << endl;
        }
    }else if(cmd == CMD_SHORTDATA){
            qDebug() << "receive data :  " << payload << endl;
            //maybe emit the data to the main thread or other operation
            emit signalReceived(cmd,payload);
    }
}


//多包数据处理入口
void CommunicationPort::onDealCompletePacketData(REPORT_COMMAND cmd,const QByteArray &data)
{
    //emit signalReceived(cmd,data);
    //同样分类处理多包传输的数据
    if(cmd == CMD_PICTURETRANSMIT)
    {
        //移除结束标志
        QByteArray recvData = data;
        recvData.chop(5);
        //例如是张图片
        QImage image;
        if (!image.loadFromData(data)) {
            qDebug() << "Error: Cannot load image from byte array.";
            return;
        }
        QString savePath = QCoreApplication::applicationDirPath() + "/images/test1.png";
        if (image.save(savePath)) {
                qDebug() << "Image saved successfully to:" << savePath;
                emit signalReceived(CMD_PICTURETRANSMIT,savePath.toUtf8());
            } else {
                qDebug() << "Error: Failed to save image to" << savePath;
            }
    }
}

void CommunicationPort::onCheckError(REPORT_COMMAND cmd)
{
    qDebug() << "frame check error" << cmd << endl;
}

int CommunicationPort::getPortType()
{
    return m_portType;
}

void CommunicationPort::close()
{
    emit signalClose();
}

void CommunicationPort::bindPort(int port)
{
    emit signalBindPort(port);
}

void CommunicationPort::open(const QString &strAddress, const int &number)
{
    emit signalOpen(strAddress,number);
}

void CommunicationPort::write(const QByteArray &data)
{
    emit signalWrite(data);
}
