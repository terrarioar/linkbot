#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QGamepad>
#include <QGamepadManager>
#include <QMutex>
#include <QMutexLocker>

#include <QtGui/QPixmap>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "video.h"
#include <QThread>
#include <QDebug>
#include <qapplication.h>

namespace Ui {
    class MainWindow;
}

using namespace std;
using namespace cv;

class MainWindow;
class videothread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    void initUi();
    void initCam();
    void initGamepad();
    void initConnect();
    void clearscene();
    void openCam();
    void sendDataProcess(QString command);
    void disconnectPort();
    void transformCurrentBtn(bool pressed, QString name, int index);
    void transformBtnVaule(int btnName, double value);
    QList<QSerialPortInfo> m_portInfo;  // 可用端口列表
    QList<QString> m_portNames;         // 可用端口列表
    QSerialPort* m_pSerial;             // 连接串口端口
    //QTimer m_timer;                     // 更新当前系统时间
    QTimer m_gamecost;                  // 定时检测手柄连接
    QTimer m_serialPortTimer;           // 定时检测端口是否连接
    QString m_textData;                 // 文本数据缓冲区
    QString m_hexData;                  // 十六进制数据缓冲区
    bool linkState = false;             // 端口连接状态
    bool gamepadLinkState = false;      // 手柄连接状态
    bool defParaState = true;           // 默认参数状态
    enum mode { inplace = 0, goforward = 1, retreat = 2 };                      // 限定运动模式
    mode modestate = inplace;                     // 机器人运动模式
    qint64 receiveByte = 0;             // 记录接收字节数
    qint64 sendByte = 0;                // 记录发送字节数
    QGamepad* m_gamepad;                // 连接手柄
    
    QGraphicsScene *scene_1,*scene_2;
    bool reset = false;

private slots:
    void updataPortNum();
    //void updateLocalTime();
    void checkConnectState();
    void checkTimeout();
    void handleSerialError(QSerialPort::SerialPortError error);
    void onOpenGamepad();
    void onOpenPort();
    void onSendData();
    void onReadData();
    void changeLed1Light();
    void changeLed2Light();
    void changeLed3Light();
    //void onTimedData(bool checked);
    //void onReceiveAreaStyle(bool checked);
    void onChangeReceivedDataFormat(bool checked);
    void onClearSendData();
    void onClearReceivedData();
    void onStopDisplay(bool checked);
    //void onTimeCycleTextFormat();
    void modifySendDataFormatSlot();
    void sendDataChangeSlot();
    void goForward();
    void retreatMode();
    void stop();
    void upliftHead();
    void upliftTail();
    void bowHead();
    void bowTail();
    void bowHead_2();
    void bowTail_2();
    void turnLeft();
    void turnRight();
    void turnMode();
    void defParaMode();
    void show_image_1(QImage dstImage);  //分别在两个graphview中显示图像
    void show_image_2(QImage dstImage);
    void on_rstpushbutton_clicked();
    void on_flippushbutton_clicked();
    void on_openpushbutton_clicked();
    void on_closepushbutton_clicked();

signals:
    int signal_flip_1();
    int signal_flip_2();
    int signal_openrecord_1();
    int signal_openrecord_2();
    int signal_endrecord_1();
    int signal_endrecord_2();
};

class videothread : public QThread
{
    Q_OBJECT

public:
    videothread(int cam, int order);
    ~videothread();
    void run();   //qt多线程类需要重载run函数
    /*
     * 多线程类中的run函数类似于主线程的main函数，
     * 例如创建多线程类对象：videothread th1;   th1.start()之后便自动开始执行run函数
     */

private:
    int cam;     //打开摄像头的索引
    int order;
    VideoWriter* writer;
    bool record_flag = false;
    bool flip_flag = false;
    bool m_closeflag = false;
    QMutex m_mutex;

public slots:
    void flipmat();
    void openrecord();
    void endrecord();
    void closeCam();

signals:
    /* 使用多线程可以同时读取两个摄像头的图像
     * 但是在进行显示的时候需要操控UI界面中的控件，然而这一操作必须在主线程里执行
     * 换言之，子线程不能对UI界面的控件进行修改
     * 解决办法是当子线程读取图像以后，向主线程发送信号，并且将图像数据传送过去，然后由主线程中的槽函数执行显示图像的函数
     */
    int signal_tomainthread(QImage dstImage);

};

QImage cvMat2QImage(const Mat& mat);   //opencv的Mat数据类型转换为qt的QImage数据类型

#endif // MAINWINDOW_H
