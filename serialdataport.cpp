#include "serialdataport.h"
#include <QDebug>
SerialDataPort::SerialDataPort(QObject *parent) : QObject(parent)
{

}

void SerialDataPort::onError(QSerialPort::SerialPortError value)
{
    if(value != 0)
    {
        QString ErrInfo;
        ErrInfo = "SerialPortError  ErrorCode:" + QString::number(value);
        emit signalError(ErrInfo);
    }
}

void SerialDataPort::onInit()
{
    m_serialPort = new QSerialPort;
    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(onRead()));
    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onError(QSerialPort::SerialPortError)));

}

void SerialDataPort::onOpen(const QString &portName, const int &baudRate)
{
    m_serialPort->setPortName(portName);
    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        m_serialPort->setBaudRate(baudRate);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        emit signalConnected();
        qDebug() << "port opened" << endl;
    }else{
        qDebug() << " serialport open fail!";
    }
}

void SerialDataPort::onRead()
{
    if (m_serialPort)
    {
       QByteArray data = m_serialPort->readAll();
       emit signalReceived(data);
       qDebug() << "serialport received : " << data.size() << data << endl;
    }
}

void SerialDataPort::onWrite(const QByteArray &data)
{
    m_serialPort->write(data);
}

void SerialDataPort::onClose()
{
    m_serialPort->close();
    emit signalDisconnected();
}
