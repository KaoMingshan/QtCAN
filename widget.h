#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "zlgcan.h"
#include <QMetaType>
#include <QCloseEvent>
#include <QObject>
#include "qReceiveMessageThread.h"

namespace Ui {
class Widget;
}

//设置设备信息
typedef struct _DeviceInfo
{
    UINT deviceName; //设备名称
    UINT channelCount; //通道个数
}DeviceInfo;

//列表数据与对话框数据一一对应
static const DeviceInfo deviceType[] =
{
    {ZCAN_USBCAN1, 1},
    {ZCAN_USBCAN2, 2},
    {ZCAN_PCI9820I,2},
    {ZCAN_USBCAN_E_U, 1},
    {ZCAN_USBCAN_2E_U, 2},
    {ZCAN_USBCAN_4E_U, 4},
    {ZCAN_PCIE_CANFD_100U, 1},
    {ZCAN_PCIE_CANFD_200U, 2},
    {ZCAN_PCIE_CANFD_400U_EX, 4 },
    {ZCAN_USBCANFD_200U, 2},
    {ZCAN_USBCANFD_100U, 1},
    {ZCAN_USBCANFD_MINI, 1},
    {ZCAN_CANETTCP, 1},
    {ZCAN_CANETUDP, 1},
    {ZCAN_WIFICAN_TCP, 1},
    {ZCAN_WIFICAN_UDP, 1},
    {ZCAN_CLOUD, 1},
    {ZCAN_CANFDWIFI_TCP, 1},
    {ZCAN_CANFDWIFI_UDP, 1},
    {ZCAN_CANFDNET_TCP, 2},
    {ZCAN_CANFDNET_UDP, 2},
    {ZCAN_CANFDNET_400U_TCP, 4},
    {ZCAN_CANFDNET_400U_UDP, 4},
};

//USBCANFD
static const UINT kAbitTimingUSB[] =
{
    1000000,//1Mbps
    800000,//800kbps
    500000,//500kbps
    250000,//250kbps
    125000,//125kbps
    100000,//100kbps
    50000 //50kbps
};

static const UINT kDbitTimingUSB[] =
{
    5000000,//5Mbps
    4000000,//4Mbps
    2000000,//2Mbps
    1000000 //1Mbps
};

//PCIECANFD brp=1
static const UINT kAbitTimingPCIE[] = {
    1000000, //1M(80%)
    800000, //800K(80%)
    500000, //500K(80%)
    250000, //250K(80%)
    125000  //125K(80%)
};

static const UINT kDbitTimingPCIE[] = {
    8000000, //8Mbps(80%)
    5000000, //5Mbps(75%)
    5000000, //5Mbps(87.5%)
    4000000, //4Mbps(80%)
    2000000, //2Mbps(80%)
    1000000  //1Mbps(80%)
};

static const unsigned kBaudrate[] = {
    1000000,
    800000,
    500000,
    250000,
    125000,
    100000,
    50000,
    20000,
    10000,
    5000
};


class Widget : public QWidget
{
    Q_OBJECT

private:
    /* 设备选择 */
    int deviceTypeIndex; //设备
    int deviceIndex; //设备索引
    int channelIndex; //通道索引

    /* 参数设置区域 */
    int workModeIndex; //工作模式索引
    bool resistanceEnable; //终端电阻使能
    int baudIndex; //波特率
    int filterModeIndex; // 滤波模式
    int arbitBaudIndex; //仲裁波特率
    int dataBaudIndex; //数据波特率
    QString accCode; //验收码
    QString shieldCode; //屏蔽码

    /* 数据发送区域 */
    QString id; //报文id
    int frameTypeIndex; //帧类型
    int protocalTypeIndex; //协议类型
    int canfdAccIndex; //CANFD 加速
    QString delayTime; //延时
    bool queueFrameMark; //队列帧延时标记
    QString dataSend; //发送的数据
    int sendTypeIndex; //发送方式
    QString onceSendFrame; //一次性发送帧数

    /* 队列发送区 */
    bool    supportDelaySend;  //设备是否支持队列发送
    bool    supportDelaySendMode;  //设备队列发送是否需要设置队列发送模式,USBCANFD系列，PCIECANFD系列设备需要设置发送模式才可以进行队列发送
    bool    supportGetSendMode;  //设备是否支持查询当前模式

    /* 定时发送区 */
    int autoSendIndex; //定时发送索引
    int autoPeriodIndex; //定时间隔

    DEVICE_HANDLE deviceHandle; //设备句柄
    CHANNEL_HANDLE channelHandel; //通道句柄

    bool isOpenDevice; //设备是否打开
    bool isCANStart; //CAN是否启动

    IProperty * property_; //属性

    QReceiveMessageThread  threadRecMsg;//消息接收线程

    /* 数据接收 */

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void closeEvent(QCloseEvent * closeevent);

private:
    Ui::Widget *ui;

private:
    void initComboxIndex(QObject * obj, int begin, int end, int currentIndex);
    bool setBaudRate(); //设置CAN卡波特率
    bool setCanfdBaudRate(); //设置CANFD卡波特率
    bool setResistanceEnable(); //设置终端电阻使能
    void enableCtrl(bool opened); //combobox使能控制
};

#endif // WIDGET_H
