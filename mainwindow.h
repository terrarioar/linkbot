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
    QList<QSerialPortInfo> m_portInfo;  // ���ö˿��б�
    QList<QString> m_portNames;         // ���ö˿��б�
    QSerialPort* m_pSerial;             // ���Ӵ��ڶ˿�
    //QTimer m_timer;                     // ���µ�ǰϵͳʱ��
    QTimer m_gamecost;                  // ��ʱ����ֱ�����
    QTimer m_serialPortTimer;           // ��ʱ���˿��Ƿ�����
    QString m_textData;                 // �ı����ݻ�����
    QString m_hexData;                  // ʮ���������ݻ�����
    bool linkState = false;             // �˿�����״̬
    bool gamepadLinkState = false;      // �ֱ�����״̬
    bool defParaState = true;           // Ĭ�ϲ���״̬
    enum mode { inplace = 0, goforward = 1, retreat = 2 };                      // �޶��˶�ģʽ
    mode modestate = inplace;                     // �������˶�ģʽ
    qint64 receiveByte = 0;             // ��¼�����ֽ���
    qint64 sendByte = 0;                // ��¼�����ֽ���
    QGamepad* m_gamepad;                // �����ֱ�
    
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
    void show_image_1(QImage dstImage);  //�ֱ�������graphview����ʾͼ��
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
    void run();   //qt���߳�����Ҫ����run����
    /*
     * ���߳����е�run�������������̵߳�main������
     * ���紴�����߳������videothread th1;   th1.start()֮����Զ���ʼִ��run����
     */

private:
    int cam;     //������ͷ������
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
    /* ʹ�ö��߳̿���ͬʱ��ȡ��������ͷ��ͼ��
     * �����ڽ�����ʾ��ʱ����Ҫ�ٿ�UI�����еĿؼ���Ȼ����һ�������������߳���ִ��
     * ����֮�����̲߳��ܶ�UI����Ŀؼ������޸�
     * ����취�ǵ����̶߳�ȡͼ���Ժ������̷߳����źţ����ҽ�ͼ�����ݴ��͹�ȥ��Ȼ�������߳��еĲۺ���ִ����ʾͼ��ĺ���
     */
    int signal_tomainthread(QImage dstImage);

};

QImage cvMat2QImage(const Mat& mat);   //opencv��Mat��������ת��Ϊqt��QImage��������

#endif // MAINWINDOW_H
