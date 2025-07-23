#ifndef COMMAND_H
#define COMMAND_H

enum PortType{
    NULL_PORT,
    UDP_PORT,
    SERIAL_PORT
};

//命令
typedef enum _Report_Command{
    CMD_PICTURETRANSMIT,//需要分包传输的图片
    CMD_SHORTDATA,//短数据
    CMD_UNKNOWN //未知
}REPORT_COMMAND;


#endif // COMMAND_H
