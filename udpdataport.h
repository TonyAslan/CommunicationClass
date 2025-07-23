#ifndef UDPDATAPORT_H
#define UDPDATAPORT_H

#include <QObject>
#include <QUdpSocket>
#include <QMutex>
class UdpDataPort : public QObject
{
    Q_OBJECT
public:
    explicit UdpDataPort(QObject *parent = nullptr);

signals:
    void signalReceived(const QByteArray& data);
    void signalError(QString);
    void signalConnected();
    void signalDisconnected();
public slots:
    void onError(QAbstractSocket::SocketError);
    void onInit();
    void onOpen(const QString& address, const int& port);
    void onRead();
    void onWrite(const QByteArray& data);
    void onClose();
    void onBindPort(int port);
private:
    QUdpSocket* m_socket;
    mutable QMutex m_mutex;
};

#endif // UDPDATAPORT_H
