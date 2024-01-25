#ifndef QRECEIVEMESSAGETHREAD_H
#define QRECEIVEMESSAGETHREAD_H

#include "zlgcan.h"
#include <QMetaType>
#include <QObject>
#include <QThread>

class QReceiveMessageThread : public QThread
{
    Q_OBJECT
private:
    bool m_pause = true;    // 暂停
    bool m_stop = false;    // 停止
private:
    CHANNEL_HANDLE channel_handle_;// CAN通道句柄
protected:
    void run() Q_DECL_OVERRIDE; // 线程任务
public:
    QReceiveMessageThread();

    void beginThread();
    void pauseThread();
    void stopThread();
signals:
    void newMsg(ZCAN_Receive_Data *can_data, uint len);
    void newMsg(ZCAN_ReceiveFD_Data *canfd_data, uint len);
private slots:
    void slot_deviceInfo(CHANNEL_HANDLE channel_handle);
};

#endif // QRECEIVEMESSAGETHREAD_H
