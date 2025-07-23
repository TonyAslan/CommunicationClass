#include "udpdataport.h"
#include <QDebug>
UdpDataPort::UdpDataPort(QObject *parent) : QObject(parent)
{

}

void UdpDataPort::onError(QAbstractSocket::SocketError value)
{
    QString ErrInfo;
    ErrInfo = "SocketError  ErrorCode:" + QString::number(value);
    emit signalError(ErrInfo);
}

void UdpDataPort::onInit()
{
    m_socket = new QUdpSocket;
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onRead()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(connected()), this, SIGNAL(signalConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(signalDisconnected()));
}

void UdpDataPort::onOpen(const QString &address, const int &port)
{
    m_socket->abort();
    m_socket->connectToHost(address, port);
    qDebug() << "port opened" << endl;
}

void UdpDataPort::onRead()
{
    if(m_socket)
    {
        while (m_socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(m_socket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            qint64 bytesRead = m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            if (bytesRead == -1) {
                qDebug() << "socket read fail : " << m_socket->errorString();
            }
            emit signalReceived(datagram);
        }
    }
}

void UdpDataPort::onWrite(const QByteArray &data)
{
    m_socket->write(data);
}

void UdpDataPort::onClose()
{
    m_socket->disconnectFromHost();
}

void UdpDataPort::onBindPort(int port)
{
    if(m_socket == nullptr)
        return;

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->close();

    if (m_socket->bind(QHostAddress::Any,port))
    {
        qDebug() << "bind success" << endl;
    }else {
        qDebug() << "bind fail" << endl;
    }
}
