#ifndef COMMUNICATIONPORT_H
#define COMMUNICATIONPORT_H

#include <QObject>
#include <QMutex>
#include "command.h"

class SerialDataPort;
class UdpDataPort;
class ProtocolHandler;

class CommunicationPort : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationPort(QObject *parent = nullptr) = delete;
    CommunicationPort(PortType type,QObject *parent = nullptr);
    ~CommunicationPort();

    void sendDatas(REPORT_COMMAND cmd, const QByteArray& data);

    //获取当前端口类别
    int getPortType();
    //打开 串口：串口号、波特率 网络：地址、端口
    void open(const QString& strAddress, const int& number);

    void close();
    //作为网络接收端时 需要绑定端口
    void bindPort(int port);

private:
    void write(const QByteArray& data);

    // 处理有效载荷
    void handleDecodedPayload(REPORT_COMMAND cmd, const QByteArray& payload);


private slots:
    //接收到数据
    void onReceiveDatas(const QByteArray &rawData);
    //处理完整的分包数据
    void onDealCompletePacketData(REPORT_COMMAND cmd, const QByteArray& data);
    //校验错误
    void onCheckError(REPORT_COMMAND cmd);
signals:
    //对外
    void signalReceived(REPORT_COMMAND cmd,const QByteArray& data);
    void signalError(QString);
    void signalOpened();
    void signalClosed();
    //对内
    void signalWrite(const QByteArray& data);
    void signalOpen(QString str, int number);
    void signalClose();
    void signalQuiting();
    void signalBindPort(int);

private:
    QThread* m_thread;
    PortType m_portType;
    UdpDataPort* m_udpDataPort;
    SerialDataPort* m_serialDataPort;
    ProtocolHandler* m_protocolHandler;
    //接收缓冲区
    QByteArray m_receiveBuffer;
};

#endif // COMMUNICATIONPORT_H
