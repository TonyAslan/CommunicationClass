#include "protocolhandler.h"
#include <QDebug>
ProtocolHandler::ProtocolHandler(QObject* parent) : QObject(parent) {}
//创建帧 参数: 指令 有效载荷
QByteArray ProtocolHandler::createFrame(REPORT_COMMAND cmd, const QByteArray& payload) {
    QByteArray frame;
    frame.append("$$");                            // 帧头
    frame.append(static_cast<char>(cmd));          // 命令字
    frame.append(static_cast<char>(payload.size() >> 8));  // 长度高字节
    frame.append(static_cast<char>(payload.size() & 0xFF));// 长度低字节
    //第六个字节开始是有效载荷
    frame.append(payload);                         // 有效载荷
    //mid(2) 从2开始，提取到数组末尾
    quint16 checksum = calculateChecksum(frame.mid(2)); // 计算校验（跳过帧头）
    frame.append(static_cast<char>(checksum >> 8));   //校验和高字节
    frame.append(static_cast<char>(checksum & 0xFF)); //校验和低字节
    frame.append('#');                                  // 帧尾
    qDebug() << "create frame : " << frame << endl;
    return frame;
}
//校验合法帧 提取有效载荷  参数：原始数据 有效载荷
bool ProtocolHandler::validateFrame(const QByteArray& rawData, QByteArray& outPayload) {
    //qDebug() << "validateFrame rawData : " << rawData << "size : "<< rawData.size() << endl;
    // 检查最小长度：帧头(2) + 命令(1) + 长度(2) + 校验(2) + 帧尾(1) = 8字节
    if (rawData.size() < 8 || rawData.left(2) != "$$" || rawData.right(1) != "#") {
        //emit signalChecksumError(CMD_UNKNOWN);
        return false;
    }

    // 提取校验和
    quint16 receivedChecksum = (static_cast<quint8>(rawData[rawData.size()-3]) << 8)
                             | static_cast<quint8>(rawData[rawData.size()-2]);

    // 计算实际校验和
    QByteArray checkData = rawData.mid(2, rawData.size() - 5); // 去除头、校验、尾
    quint16 actualChecksum = calculateChecksum(checkData);

    if (receivedChecksum != actualChecksum) {
        emit signalChecksumError(static_cast<REPORT_COMMAND>(rawData[2]));
        return false;
    }

    // 提取有效载荷
    int payloadLength = (static_cast<quint8>(rawData[3]) << 8) | static_cast<quint8>(rawData[4]);
    outPayload = rawData.mid(5, payloadLength); // 命令字后是有效载荷
    //qDebug() << "payload : " << outPayload << endl;
    return true;
}

//处理分包传输的情况---------------------------------------------------------------

// 分包传输初始化
void ProtocolHandler::initPacketTransmission(const QByteArray& fullData, int maxPacketSize, REPORT_COMMAND cmd) {
    PacketState state; //分包状态
    state.fullData = fullData; //完整数据
    state.currentOffset = 0; //当前偏移
    state.maxPacketSize = maxPacketSize; //最大分包大小
    state.totalPackets = (fullData.size() + maxPacketSize - 1) / maxPacketSize; //总包数
    m_packetStates[cmd] = state;
}
//获取下一包
bool ProtocolHandler::getNextPacket(REPORT_COMMAND cmd, QByteArray& packet) {
    auto& state = m_packetStates[cmd];
    //没有下一包了
    if (state.currentOffset >= state.fullData.size()) return false;
    //剩余
    int remaining = state.fullData.size() - state.currentOffset;
    //块大小
    int chunkSize = qMin(remaining, state.maxPacketSize);
    //从完整数据中截取一个包
    packet = state.fullData.mid(state.currentOffset, chunkSize);
    //偏移量增加
    state.currentOffset += chunkSize;
    return true;
}

int ProtocolHandler::getCurrentOffset(REPORT_COMMAND cmd)
{
    PacketState state = m_packetStates.value(cmd);
    return state.currentOffset;
}

//多包传输情况下接受包
bool ProtocolHandler::processReceivedPacket(REPORT_COMMAND cmd, const QByteArray& packet) {
    //缓存
    static QByteArray assembledData;
    assembledData.append(packet);
    //收到结束标记，传输完成
    if (packet.endsWith("#END#")) {
        qDebug() << "multipack transform end!" << endl;
        emit signalPacketTransmissionComplete(cmd, assembledData);
        assembledData.clear();
        return true;
    }
    return false;
    qDebug() << " ProtocolHandler::processReceivedPacket false" << endl;
}

//检验和
quint16 ProtocolHandler::calculateChecksum(const QByteArray& data) {
    quint16 sum = 0;
    for (auto ch : data) {
        sum += static_cast<quint8>(ch);
    }
    return sum;
}
