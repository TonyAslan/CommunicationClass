#ifndef SERIALDATAPORT_H
#define SERIALDATAPORT_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>

class SerialDataPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialDataPort(QObject *parent = nullptr);

signals:
    void signalReceived(const QByteArray& data);
    void signalError(QString);
    void signalConnected();
    void signalDisconnected();
public slots:
    void onError(QSerialPort::SerialPortError);
    void onInit();
    void onOpen(const QString& portName, const int& baudRate);
    void onRead();
    void onWrite(const QByteArray& data);
    void onClose();
private:
    QSerialPort* m_serialPort;
    mutable QMutex m_mutex;
};

#endif // SERIALDATAPORT_H
