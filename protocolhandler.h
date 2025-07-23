#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

// protocolhandler.h
#pragma once
#include <QObject>
#include <QByteArray>
#include <QMap>

// 协议命令字枚举
#include "command.h"
//将分包传输这部分封装一下
class ProtocolHandler : public QObject {
    Q_OBJECT
public:
    explicit ProtocolHandler(QObject* parent = nullptr);

    //---------------- 协议封装 -----------------
    // 生成完整数据帧（含校验）
    QByteArray createFrame(REPORT_COMMAND cmd, const QByteArray& payload);

    // 校验数据帧完整性（返回校验结果和有效载荷）
    bool validateFrame(const QByteArray& rawData, QByteArray& outPayload);

    //---------------- 分包管理 -----------------
    // 初始化分包传输
    void initPacketTransmission(const QByteArray& fullData, int maxPacketSize,REPORT_COMMAND cmd);

    // 获取下一个要发送的分包（返回是否还有后续包）
    bool getNextPacket(REPORT_COMMAND cmd, QByteArray& packet);

    //当前指令发送的数据偏移量
    int getCurrentOffset(REPORT_COMMAND cmd);

    // 处理接收到的分包（返回是否完成）
    bool processReceivedPacket(REPORT_COMMAND cmd, const QByteArray& packet);

signals:
    // 分包传输完成
    void signalPacketTransmissionComplete(REPORT_COMMAND cmd, const QByteArray& fullData);
    // 校验错误通知
    void signalChecksumError(REPORT_COMMAND cmd);

private:
    // 计算校验和（私有工具函数）
    quint16 calculateChecksum(const QByteArray& data);

    // 分包状态管理
    struct PacketState {
        QByteArray fullData;      // 完整数据
        int currentOffset;        // 当前偏移量
        int maxPacketSize;        // 最大分包大小
        int totalPackets;         // 总包数
    };
    //管理指令和分包状态
    QMap<REPORT_COMMAND, PacketState> m_packetStates;
};

#endif // PROTOCOLHANDLER_H
