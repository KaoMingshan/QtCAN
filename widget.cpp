#include "widget.h"
#include "ui_widget.h"

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

    //initComboxIndex(ui->DeviceIndexCombox,0,32,0);
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
