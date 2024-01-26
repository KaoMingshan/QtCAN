#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),

  //设置默认信息
     deviceTypeIndex(0),
     deviceIndex(0),
     channelIndex(0),
     workModeIndex(0),
     resistanceEnable(true),
     baudIndex(0),
     filterModeIndex(1),
     arbitBaudIndex(0),
     dataBaudIndex(0),
     accCode("00000000"),
     shieldCode("FFFFFFFF"),
     id("00000001"),
     frameTypeIndex(0),
     protocalTypeIndex(1),
     canfdAccIndex(1),
     delayTime(1000),
     queueFrameMark(false),
     dataSend("00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63"),
     sendTypeIndex(0),
     onceSendFrame(10),
     autoSendIndex(0),
     autoPeriodIndex(1000)

{
    ui->setupUi(this);

    //初始化相关的数据
    isOpenDevice = false;
    isCANStart = false;
    property_ = NULL;
    supportDelaySend = false;
    supportDelaySendMode = false;
    supportGetSendMode = false;

    initComboxIndex(ui->DeviceIndexCombox,0,32,0);

    connect(&threadRecMsg, SIGNAL(started()),this,SLOT(slotThreadrecmsgStarted()));
    connect(&threadRecMsg, SIGNAL(finished()),this, SLOT(slotThreadrecmsgFinished()));
    connect(&threadRecMsg, (void (QReceiveMessageThread:: *)(ZCAN_Receive_Data *, uint))&QReceiveMessageThread::newMsg, this, (void (Widget:: *)(ZCAN_Receive_Data *, uint))&Widget::slotThreadrecmsgNewmsg);
    connect(&threadRecMsg, (void (QReceiveMessageThread:: *)(ZCAN_ReceiveFD_Data *, uint))&QReceiveMessageThread::newMsg, this, (void (Widget:: *)(ZCAN_ReceiveFD_Data *, uint))&Widget::slotThreadrecmsgNewmsg);


    connect(this,SIGNAL(deviceInfo(CHANNEL_HANDLE)),&threadRecMsg,SLOT(slot_deviceInfo(CHANNEL_HANDLE)));
}

Widget::~Widget()
{
    delete ui;
}

//关闭窗口时结束进程
void Widget::closeEvent(QCloseEvent *closeevent)
{
    if(threadRecMsg.isRunning())
        {
            threadRecMsg.stopThread();
            threadRecMsg.wait();
        }
    closeevent->accept();
}

//在开始时初始化combobox的下拉框
void Widget::initComboxIndex(QObject *obj, int begin, int end, int currentIndex)
{
    QComboBox * combox = static_cast<QComboBox*>(obj); //进行类型转换
    Q_ASSERT(combox != NULL); //添加断言进行判断
    combox->clear();

    for(int i = begin; i < end; ++i)
    {
        combox->addItem(QString::asprintf("%d",i));
    }
    combox->setCurrentIndex(currentIndex);
}

//设置CAN波特率   ZCAN_SetValue(device_handle,path,value)-->P48;
bool Widget::setBaudRate()
{
    char path[50] = {0};
    sprintf_s(path,"%d/baud_rate",channelIndex);
    char value[10] = {0};
    sprintf_s(value,"%d",kBaudrate[baudIndex]);
    return 1 == ZCAN_SetValue(deviceHandle,path,value);
}


//设置canfd波特率
bool Widget::setCanfdBaudRate()
{
    char path[50] = {0};
    sprintf_s(path,"%d/canfd_abit_baud_rate",channelIndex);
    char value[10] = {0};
    sprintf_s(value,"%d",kAbitTimingUSB[arbitBaudIndex]);
    int ret_a = ZCAN_SetValue(deviceHandle,path,value);

    sprintf_s(path,"%d/canfd_dbit_baud_rate",channelIndex);
    sprintf_s(value,"%d",kDbitTimingUSB[dataBaudIndex]);
    int ret_b = ZCAN_SetValue(deviceHandle,path,value);

    return 1 == (ret_b && ret_a);
}

//设置终端电阻使能
bool Widget::setResistanceEnable()
{
    char path[50] = {0};
    sprintf_s(path,"%d/initenal_resistance",channelIndex);
    char value[10] = {0};
    sprintf_s(value,"%d",resistanceEnable);
    return 1 == ZCAN_SetValue(deviceHandle,path,value);
}

void Widget::enableCtrl(bool opened)
{
    ui->DeviceCombox->setEnabled(!opened);
    ui->DeviceIndexCombox->setEnabled(!opened);
    ui->ChannelIndexCombox->setEnabled(!opened);
    ui->OpenDeviceButton->setEnabled(!opened);
}

bool Widget::IsNetCAN(uint type)
{
    return (type==ZCAN_CANETUDP || type==ZCAN_CANETTCP || type==ZCAN_WIFICAN_TCP || type==ZCAN_WIFICAN_UDP ||
            type==ZCAN_CANDTU_NET || type==ZCAN_CANDTU_NET_400);
}

bool Widget::IsNetCANFD(uint type)
{
    return (type==ZCAN_CANFDNET_TCP || type==ZCAN_CANFDNET_UDP ||
           type==ZCAN_CANFDNET_400U_TCP || type==ZCAN_CANFDNET_400U_UDP ||
           type==ZCAN_CANFDWIFI_TCP || type==ZCAN_CANFDWIFI_UDP);
}

bool Widget::IsNetTCP(uint type)
{
    return (type==ZCAN_CANETTCP || type==ZCAN_WIFICAN_TCP || type==ZCAN_CANDTU_NET || type==ZCAN_CANDTU_NET_400 ||
            type==ZCAN_CANFDNET_TCP || type==ZCAN_CANFDNET_400U_TCP ||type==ZCAN_CANFDWIFI_TCP );

}

bool Widget::IsNetUDP(uint type)
{
    return (type==ZCAN_CANETUDP || type==ZCAN_WIFICAN_UDP ||
            type==ZCAN_CANFDNET_UDP || type==ZCAN_CANFDNET_400U_UDP ||type==ZCAN_CANFDWIFI_UDP);
}

void Widget::setCtrlStateDelaySend(bool delaySend, bool delaySendMode, bool getSendMode)
{
    ui->QueueFrameDelayCkb->setEnabled(delaySend);
    ui->QueueSendModelCkb->setEnabled(delaySend && delaySendMode);
    ui->GetQueueAvaiButon->setEnabled(delaySend);
    ui->ClearQueueButton->setEnabled(delaySend);
    ui->FindDeviceModelButton->setEnabled(delaySend && getSendMode);
}


void Widget::setAutosendCtrlState(bool supportCan, bool supportCanfd, bool supportIndex, bool supportSingleCancel, bool supportGetAutosendList)
{
    bool supportAutosend = supportCan || supportCanfd;
    ui->IndexSpinBox->setEnabled(false);
    ui->IntervalTimeEdit->setEnabled(supportAutosend);
    ui->GetQueueAvaiButon->setEnabled(supportAutosend);
    ui->OpenIntervalSendButton->setEnabled(supportAutosend);
    ui->CloseIntervalSendButton->setEnabled(supportAutosend);
    ui->CancelSingleSendBtn->setEnabled(supportSingleCancel);
    ui->SelectTimeListBtn->setEnabled(supportGetAutosendList);
}


//设备combobox更改后进行更新
void Widget::on_DeviceCombox_currentIndexChanged(int index)
{
    deviceTypeIndex = index;
    //通过设备类型来选择通道
    initComboxIndex(ui->ChannelIndexCombox,0,deviceType[deviceTypeIndex].channelCount, 0);

    uint type = deviceType[deviceTypeIndex].deviceName; //获取设备名称
    const bool cloudDevice = type = ZCAN_CLOUD;
    const bool netcanfd = IsNetCANFD(type);
    const bool netcan = IsNetCAN(type);
    const bool netDevice = (netcan || netcanfd);
    const bool tcpDevice = IsNetTCP(type);
    const bool usbcanfd = type==ZCAN_USBCANFD_100U ||
            type==ZCAN_USBCANFD_200U || type==ZCAN_USBCANFD_MINI;
    const bool pciecanfd = type==ZCAN_PCIE_CANFD_100U ||
            type==ZCAN_PCIE_CANFD_200U || type==ZCAN_PCIE_CANFD_400U|| type==ZCAN_PCIE_CANFD_400U_EX;

    const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
    const bool accFilter = pciecanfd || type==ZCAN_USBCAN1 || type==ZCAN_USBCAN2;

    //队列发送支持
    supportDelaySend = usbcanfd || pciecanfd || netcanfd;
    supportDelaySendMode = usbcanfd || pciecanfd;
    supportGetSendMode = usbcanfd || pciecanfd;
    setCtrlStateDelaySend(supportDelaySend, supportDelaySendMode,supportGetSendMode);
    ui->QueueFrameDelayCkb->setChecked(Qt::Unchecked);

    //定时发送支持  检查CANFD设备、其他CAN设备、PCIECANFD设备和USB/NETCANFD设备，并根据其支持情况设置自动发送的控制状态
    const bool supportAutosendCanfd = canfdDevice; //CANFD设备
    const bool supportAutosendCan = canfdDevice; //CANFD设备和其他设备
     // PCIECANFD 不支持使用索引控制定时，PCIECANFD添加一条即立即发送
    const bool supportAutosendIndex = (supportAutosendCan && !pciecanfd);
    const bool supportStopSingleAutosend = usbcanfd;
    const bool supportGetAutosendList = netcanfd;
    setAutosendCtrlState(supportAutosendCan,supportAutosendCanfd,supportAutosendIndex,supportStopSingleAutosend,supportGetAutosendList);

    QStringList strList;
    if(usbcanfd)
    {
        ui->ArbitBaudCombox->clear();

        strList << "1Mbps" << "800kbps" << "500kbps" << "250kbps" << "125kbps" << "100kbps" << "50kbps";
        ui->ArbitBaudCombox->addItems(strList);

        strList.clear();
        ui->DataBaudCombox->clear();
        strList << "5Mbps" << "4Mbps" << "2Mbps" << "1Mbps";
        ui->DataBaudCombox->addItems(strList);
    }
    else if(pciecanfd)
    {
        ui->ArbitBaudCombox->clear();
        strList.clear();
        strList << "1Mbps(80%)" << "800kbps(80%)" << "500kbps(80%)" << "250kbps(80%)" << "125kbps(80%)" << "100kbps" << "50kbps";
        ui->ArbitBaudCombox->addItems(strList);

        strList.clear();
        ui->DataBaudCombox->clear();
        strList << "5Mbps(80%)" << "4Mbps(80%)" << "2Mbps(80%)" << "1Mbps(80%)";
        ui->DataBaudCombox->addItems(strList);
    }

    ui->ModelCombox->setEnabled(!cloudDevice && !netDevice);
    ui->TerminalCheckBox->setEnabled(usbcanfd);
    ui->ArbitBaudCombox->setEnabled(canfdDevice && !netDevice && !cloudDevice);
    ui->DataBaudCombox->setEnabled(canfdDevice && !netDevice && !cloudDevice);
    ui->BaudCombox->setEnabled(!canfdDevice && !netDevice && !cloudDevice);
    ui->FilterModelCombox->setEnabled(accFilter && !cloudDevice && !netDevice);
    ui->AcceptanceEdit->setEnabled(accFilter && !cloudDevice && !netDevice);
    ui->ShieldEdit->setEnabled(accFilter && !cloudDevice && !netDevice);
}


//打开设备
void Widget::on_OpenDeviceButton_clicked()
{
    //ZCAN_OpenDevice(UINT device_type, UINT device_index, UINT reserved); (设备类型， 索引号，reserved仅作保留) 该函数用于打开设备
    deviceHandle = ZCAN_OpenDevice(deviceType[deviceTypeIndex].deviceName, deviceIndex,0);
    if(INVALID_DEVICE_HANDLE == deviceHandle)
    {
        ui->DataRectextBrowser->setText("打开失败");
        return;
    }
}


//对CAN进行初始化
void Widget::on_InitCANButton_clicked()
{
    if(isOpenDevice)
    {
        ui->DataRectextBrowser->setText("设备未打开");
        return;
    }

    ZCAN_CHANNEL_INIT_CONFIG config;

    memset(&config, 0,sizeof(config));
    UINT type = deviceType[deviceTypeIndex].deviceName;
      /* 云设备和网络设备  */
//    const bool cloudDevice = type == ZCAN_CLOUD;
//    const bool netcanfd = IsNetCANFD(type);
//    const bool netcan = IsNetCAN(type);
//    const bool netDevice = (netcan || netcanfd);
//    const bool tcpDevice = IsNetTCP(type);
//    const bool server = net_mode_index_ == 0;

    //对本地设备进行连接
    const bool usbcanfd = type==ZCAN_USBCANFD_100U ||
       type==ZCAN_USBCANFD_200U || type==ZCAN_USBCANFD_MINI;
    const bool pciecanfd = type==ZCAN_PCIE_CANFD_100U ||
       type == ZCAN_PCIE_CANFD_200U || type == ZCAN_PCIE_CANFD_400U_EX;
    const bool canfdDevice = usbcanfd || pciecanfd;

    if(!canfdDevice && !setBaudRate())
    {
        ui->DataRectextBrowser->append("设置波特率失败");
        return;
    }

    if(usbcanfd)
    {
        char path[50] = {0};
        char value[100] = {0};
        sprintf_s(path,sizeof(path),"%d/canfd_standard",channelIndex);
        sprintf_s(value,sizeof(value),"%d",0);
        int ret = ZCAN_SetValue(deviceHandle,path,value);
        qDebug("%d",ret);
    }

    if(usbcanfd)
    {
        if(!setCanfdBaudRate())
        {
            ui->DataRectextBrowser->append("设置波特率失败");
            return;
        }
        config.can_type = TYPE_CANFD;
        config.canfd.mode = workModeIndex;
        config.canfd.filter = filterModeIndex;
        bool ok;
        config.canfd.acc_code = accCode.toUInt(&ok, 16);
        config.canfd.acc_mask = shieldCode.toUInt(&ok, 16);
    }
    else if(pciecanfd)
    {
        char path[50] = {0};
        char value[100] = {0};
        if(!setCanfdBaudRate())
        {
            ui->DataRectextBrowser->append("设置波特率失败");
            return;
        }

        if(type == ZCAN_PCIE_CANFD_400U_EX)
        {
            sprintf_s(path,sizeof(path),"0/set_device_recv_merge");
            sprintf_s(value,sizeof(value),"0");
            ZCAN_SetValue(deviceHandle,path,value);
        }
        config.can_type = TYPE_CANFD;
        config.canfd.mode = workModeIndex;
        config.canfd.filter = filterModeIndex;
        bool ok;
        config.canfd.acc_code = accCode.toUInt(&ok, 16);
        config.canfd.acc_mask = shieldCode.toUInt(&ok, 16);
    }

    //初始化CAN
    channelHandel = ZCAN_InitCAN(deviceHandle, channelIndex, &config);
    if(INVALID_CHANNEL_HANDLE == channelHandel)
    {
        ui->DataRectextBrowser->append("初始化CAN失败");
        return;
    }

    if(usbcanfd)
    {
        if(!resistanceEnable && !setResistanceEnable())
        {
            ui->DataRectextBrowser->append("终端使能设置失败");
            return;
        }
    }

    ui->InitCANButton->setEnabled(false);
    ui->DataRectextBrowser->append("初始化成功");
}

//启动CAN
void Widget::on_StartCANButton_clicked()
{
    if(ZCAN_StartCAN(channelHandel) != STATUS_OK)
    {
        ui->DataRectextBrowser->append("启动CAN失败");
        return;
    }

    ui->StartCANButton->setEnabled(false);
    isCANStart = true;

    emit deviceInfo(channelHandel);
    threadRecMsg.start();
    threadRecMsg.beginThread();
    ui->DataRectextBrowser->append("CAN启动成功");
}


//复位功能实现
void Widget::on_ResetButton_clicked()
{
    if(threadRecMsg.isRunning())
    {
        threadRecMsg.stopThread();
        threadRecMsg.wait();
    }

    if(ZCAN_ResetCAN(channelHandel) != STATUS_OK)
    {
        ui->DataRectextBrowser->append("复位失败");
        return;
    }

    ui->StartCANButton->setEnabled(true);
    isCANStart = false;
    ui->DataRectextBrowser->append("复位成功");
}


//关闭设备
void Widget::on_CloseDeviceButton_clicked()
{
    if(threadRecMsg.isRunning())
    {
        threadRecMsg.stopThread();
        threadRecMsg.wait();
    }
    ZCAN_ResetCAN(channelHandel);
    ZCAN_CloseDevice(deviceHandle);
    isCANStart = false;
    enableCtrl(false);
    ui->StartCANButton->setEnabled(true);
    ui->InitCANButton->setEnabled(true);
    isOpenDevice = false;
    ui->DataRectextBrowser->append("设备已经关闭");
}

//响应槽函数
void Widget::slotThreadrecmsgStarted()
{

}

void Widget::slotThreadrecmsgFinished()
{

}

void Widget::slotThreadrecmsgNewmsg(ZCAN_Receive_Data *can_data, uint len)
{
    QString qStr;
    for(uint i = 0;i < len;i++)
    {
        const ZCAN_Receive_Data &can = can_data[i];
        const canid_t & id = can.frame.can_id;
        qStr = QString::asprintf(
                    "接收到CAN ID:%08X %s %s 长度:%d 数据:", GET_ID(id), IS_EFF(id)?"扩展帧" : "标准帧",
                    IS_RTR(id)?"远程帧" : "数据帧", can.frame.can_dlc
                    );
        for(uint i = 0;i<can.frame.can_dlc;i++)
        {
            qStr.append(QString::asprintf("%02X ", can.frame.data[i]));
        }
        ui->DataRectextBrowser->append(QString(qStr));
    }
}

void Widget::slotThreadrecmsgNewmsg(ZCAN_ReceiveFD_Data *canfd_data, uint len)
{
    QString qStr;
    for(uint i = 0;i < len;i++)
    {
        const ZCAN_ReceiveFD_Data &canfd = canfd_data[i];
        const canid_t & id = canfd.frame.can_id;
        qStr = QString::asprintf(
                    "接收到CANFD ID:%08X %s %s 长度:%d 数据:", GET_ID(id), IS_EFF(id)?"扩展帧" : "标准帧",
                    IS_RTR(id)?"远程帧" : "数据帧", canfd.frame.len
                    );
        for(uint i = 0;i<canfd.frame.len;i++)
        {
            qStr.append(QString::asprintf("%02X ", canfd.frame.data[i]));
        }
        ui->DataRectextBrowser->append(QString(qStr));
    }
}



//槽函数的相关实现
void Widget::on_DeviceIndexCombox_currentIndexChanged(int index)
{
    deviceTypeIndex = index;
}

void Widget::on_ChannelIndexCombox_currentIndexChanged(int index)
{
    channelIndex = index;
}

void Widget::on_ModelCombox_currentIndexChanged(int index)
{
    workModeIndex = index;
}

void Widget::on_BaudCombox_currentIndexChanged(int index)
{
    baudIndex = index;
}

void Widget::on_TerminalCheckBox_stateChanged(int arg1)
{
    resistanceEnable = ui->TerminalCheckBox->isChecked();
}

void Widget::on_FilterModelCombox_currentIndexChanged(int index)
{
    filterModeIndex = index;
}

void Widget::on_ArbitBaudCombox_currentIndexChanged(int index)
{
    arbitBaudIndex = index;
}

void Widget::on_DataBaudCombox_currentIndexChanged(int index)
{
    dataBaudIndex = index;
}

void Widget::on_FrameTypeCombox_currentIndexChanged(int index)
{
    frameTypeIndex = index;
}

void Widget::on_AgreementCombox_currentIndexChanged(int index)
{
    protocalTypeIndex = index;
}

void Widget::on_CANFDAccCombox_currentIndexChanged(int index)
{
    canfdAccIndex = index;
}

void Widget::on_QueueFrameDelayCkb_stateChanged(int arg1)
{
    queueFrameMark = ui->QueueFrameDelayCkb->isChecked();
}

void Widget::on_SendMannerCombox_currentIndexChanged(int index)
{
    sendTypeIndex = index;
}

void Widget::on_QueueSendModelCkb_stateChanged(int arg1)
{

}

//清空实现
void Widget::on_ClearDataButton_clicked()
{
    ui->DataRectextBrowser->clear();
}
