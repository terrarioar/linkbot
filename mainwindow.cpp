#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "video.h"

#include <QMessageBox>
#include <QTextCodec>
#include <QDataStream>
#include <QListView>
#include <QProcess>
#include <QTime>
#include <QDebug>
#include <iostream>
#include <time.h>

using namespace std;


// ���VS�汾ѡ�ñ����ʽ
#if _MSC_VER > 1600
#pragma execution_character_set("utf-8")
#endif // _MSC_VER > 1600

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ��ʼ����λ��UI
    initUi();
    initCam();
}

void MainWindow::initCam() {
    videothread* th1 = new videothread(1, 1);//���������ָ�룬�����캯��ִ������Ժ�ͻ�Ѿֲ�����ɾ��
    QObject::connect(th1, SIGNAL(signal_tomainthread(QImage)),
        this, SLOT(show_image_1(QImage)));
    QObject::connect(this, SIGNAL(signal_flip_1()),
        th1, SLOT(flipmat()));
    QObject::connect(this, SIGNAL(signal_openrecord_1()),
        th1, SLOT(openrecord()));
    QObject::connect(this, SIGNAL(signal_endrecord_1()),
        th1, SLOT(endrecord()));
    // �ر�����ͷ
    QObject::connect(ui->closeCamBtn, &QPushButton::clicked,
        th1, &videothread::closeCam);
    th1->start();

    videothread* th2 = new videothread(2, 0);
    QObject::connect(th2, SIGNAL(signal_tomainthread(QImage)),
        this, SLOT(show_image_2(QImage)));
    QObject::connect(this, SIGNAL(signal_flip_2()),
        th2, SLOT(flipmat()));
    QObject::connect(this, SIGNAL(signal_openrecord_2()),
        th2, SLOT(openrecord()));
    QObject::connect(this, SIGNAL(signal_endrecord_2()),
        th2, SLOT(endrecord()));
    QObject::connect(ui->closeCamBtn, &QPushButton::clicked,
        th2, &videothread::closeCam);
    th2->start();
    // �߳̽�����ɾ��ָ��
    connect(th1, &QThread::finished, th1, &QObject::deleteLater);
    connect(th2, &QThread::finished, th2, &QObject::deleteLater);
}

// ��ʼ����λ��UI
void MainWindow::initUi()
{
    setWindowTitle("LinkBot���ƶ�");
    // ���ó�ʼ������Ϊ���׵׺���
    ui->receiveTxb->setStyleSheet("background-color: rgb(255, 255, 255);"
        "color: rgb(0, 0, 0);");

    m_pSerial = new QSerialPort;
    // ������ʾʱ��
    //connect(&m_timer, &QTimer::timeout, this, &MainWindow::updateLocalTime);
    //m_timer.start(1000);

    // ��ʱ�������ֱ�����״��
    connect(&m_gamecost, &QTimer::timeout, this, &MainWindow::checkConnectState);

    // ��ʱ�����˿��Ƿ�����
    connect(&m_serialPortTimer, &QTimer::timeout, this, &MainWindow::checkTimeout);
    m_serialPortTimer.start(1000);
    connect(m_pSerial, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error), this, &MainWindow::handleSerialError);

    // Ϊ��ʹ������Ŀ��ʽ��Ч
    ui->portNumCbx->setView(new QListView);
    ui->baudCbx->setView(new QListView);
    ui->pwmCbx->setView(new QListView);
    ui->timeCbx->setView(new QListView);

    // ������ť�����¼�ʵ�ֵ��ˢ�´���
    QObject::connect(ui->portNumCbx, SIGNAL(mouseSingleClickd()), this, SLOT(updataPortNum()));

    scene_1 = new QGraphicsScene;
    scene_2 = new QGraphicsScene;

    //��Ӷ˿ں�
    updataPortNum();
    ui->portNumCbx->setCurrentIndex(0);

    //�����ֱ�
    connect(ui->testBtn, &QPushButton::clicked, this, &MainWindow::onOpenGamepad);

    // ���Ӳۺ���
    initConnect();
}

// ����ֱ�����״̬��������ʾ
void MainWindow::checkConnectState() {
    if (m_gamepad->isConnected()) {
        ui->stateTxd->setText("Connected!");
    }
    else {
        ui->stateTxd->setText("Unconnected!");
    }
}

// ��������ֱ���Ķ���
void MainWindow::onOpenGamepad()
{
    if (gamepadLinkState == false) {
        // �����ֱ���intѡ�����ӵ��ֱ�
        m_gamepad = new QGamepad(0, this);
        if (m_gamepad->isConnected()) {
            initGamepad();
            // ��������״̬������UI
            checkConnectState();
            ui->currentBtn->setEnabled(true);
            ui->modeTxd->setEnabled(true);
            // �˶�ģʽͬ��
            if (ui->inplaceRbtn->isChecked()) {
                modestate = inplace;
                ui->modeTxd->setText("ԭ��");
            }
            else if (ui->goforwardRbtn->isChecked()) {
                modestate = goforward;
                ui->modeTxd->setText("ǰ��");
            }
            else {
                modestate = retreat;
                ui->modeTxd->setText("����");
            }
            ui->testBtn->setText("�Ͽ��ֱ�");
            gamepadLinkState = true;
        }
        else {
            delete m_gamepad;
        }
    }
    else {
        delete m_gamepad;
        m_gamecost.stop();
        ui->modeTxd->clear();
        ui->currentBtn->setEnabled(false);
        ui->modeTxd->setEnabled(false);
        ui->testBtn->setText("�����ֱ�");
        ui->stateTxd->setText("Unconnected!");
        gamepadLinkState = false;
    }
}

void MainWindow::initGamepad()
{
    // ������ʱ����������״��
    m_gamecost.start(1000);
    //����ӳ��
    connect(m_gamepad, &QGamepad::buttonSelectChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonSelect", 1);
        });
    connect(m_gamepad, &QGamepad::buttonStartChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonStart", 2);
        });
    connect(m_gamepad, &QGamepad::buttonAChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonA", 3);
        });
    connect(m_gamepad, &QGamepad::buttonBChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonB", 4);
        });
    connect(m_gamepad, &QGamepad::buttonXChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonX", 5);
        });
    connect(m_gamepad, &QGamepad::buttonYChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonY", 6);
        });
    connect(m_gamepad, &QGamepad::buttonL1Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonL1", 7);
        });
    connect(m_gamepad, &QGamepad::buttonR1Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonR1", 8);
        });
    connect(m_gamepad, &QGamepad::buttonL2Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonL2", 9);
        });
    connect(m_gamepad, &QGamepad::buttonR2Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonR2", 10);
        });
    connect(m_gamepad, &QGamepad::buttonL3Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonL3", 11);
        });
    connect(m_gamepad, &QGamepad::buttonR3Changed, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonR3", 12);
        });
    connect(m_gamepad, &QGamepad::buttonUpChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonUp", 13);
        });
    connect(m_gamepad, &QGamepad::buttonDownChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonDown", 14);
        });
    connect(m_gamepad, &QGamepad::buttonLeftChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonLeft", 15);
        });
    connect(m_gamepad, &QGamepad::buttonRightChanged, this, [=](bool pressed) {
        transformCurrentBtn(pressed, "buttonRight", 16);
        });
    connect(m_gamepad, &QGamepad::buttonL2Changed, this, [=](double value) {
        transformBtnVaule(1, value);
        });
    connect(m_gamepad, &QGamepad::buttonR2Changed, this, [=](double value) {
        transformBtnVaule(2, value);
        });
    connect(m_gamepad, &QGamepad::axisLeftXChanged, this, [=](double value) {
        transformBtnVaule(3, value);
        });
    connect(m_gamepad, &QGamepad::axisLeftYChanged, this, [=](double value) {
        transformBtnVaule(4, value);
        });
    connect(m_gamepad, &QGamepad::axisRightXChanged, this, [=](double value) {
        transformBtnVaule(5, value);
        });
    connect(m_gamepad, &QGamepad::axisRightYChanged, this, [=](double value) {
        transformBtnVaule(6, value);
        });
}


void MainWindow::initConnect()
{
    // �򿪶˿�
    connect(ui->serialStateBtn, &QPushButton::clicked, this, &MainWindow::onOpenPort);
    // ��������
    connect(ui->sendBtn, &QPushButton::clicked, this, &MainWindow::onSendData);
    // ��շ�������
    connect(ui->clearSendBtn, &QPushButton::clicked, this, &MainWindow::onClearSendData);
    // ��ս�������
    connect(ui->clearReceiveBtn, &QPushButton::clicked, this, &MainWindow::onClearReceivedData);
    // ֹͣ����������ʾ
    connect(ui->displayStateBtn, &QPushButton::clicked, this, &MainWindow::onStopDisplay);
    // ��ʱ��������
    //connect(ui->timerChk, &QCheckBox::clicked, this, &MainWindow::onTimedData);
    // �޸Ľ�����������ʽ
    //connect(ui->receiveStyleChk, &QCheckBox::clicked, this, &MainWindow::onReceiveAreaStyle);
    // �޸Ľ������ݸ�ʽ
    connect(ui->receiveFormatChk, &QCheckBox::clicked, this, &MainWindow::onChangeReceivedDataFormat);
    // �޸ķ������ݸ�ʽ
    connect(ui->sendFormatChk, &QCheckBox::clicked, this, &MainWindow::modifySendDataFormatSlot);
    // ���Ϳ����ݱ仯
    connect(ui->sendTxd, &QTextEdit::textChanged, this, &MainWindow::sendDataChangeSlot);
    // �̶�ָ��
    connect(ui->goforwardBtn, &QPushButton::clicked, this, &MainWindow::goForward);
    connect(ui->retreatBtn, &QPushButton::clicked, this, &MainWindow::retreatMode);
    connect(ui->stopBtn, &QPushButton::clicked, this, &MainWindow::stop);
    connect(ui->upliftHeadBtn, &QPushButton::clicked, this, &MainWindow::upliftHead);
    connect(ui->upliftTailBtn, &QPushButton::clicked, this, &MainWindow::upliftTail);
    connect(ui->bowHeadBtn_1, &QPushButton::clicked, this, &MainWindow::bowHead);
    connect(ui->bowTailBtn_1, &QPushButton::clicked, this, &MainWindow::bowTail);
    connect(ui->bowHeadBtn_3, &QPushButton::clicked, this, &MainWindow::bowHead_2);
    connect(ui->bowTailBtn_3, &QPushButton::clicked, this, &MainWindow::bowTail_2);
    connect(ui->turnLeftBtn, &QPushButton::clicked, this, &MainWindow::turnLeft);
    connect(ui->turnRightBtn, &QPushButton::clicked, this, &MainWindow::turnRight);
    // �˶�ģʽ�л�����
    connect(ui->inplaceRbtn, &QRadioButton::clicked, this, &MainWindow::turnMode);
    connect(ui->goforwardRbtn, &QRadioButton::clicked, this, &MainWindow::turnMode);
    connect(ui->retreatRbtn, &QRadioButton::clicked, this, &MainWindow::turnMode);
    // Ĭ�ϲ���ģʽ
    connect(ui->defParaChk, &QCheckBox::clicked, this, &MainWindow::defParaMode);
    // �л��ƹ�
    connect(ui->ledBtn_1, &QPushButton::clicked, this, &MainWindow::changeLed1Light);
    connect(ui->ledBtn_2, &QPushButton::clicked, this, &MainWindow::changeLed2Light);
    connect(ui->ledBtn_3, &QPushButton::clicked, this, &MainWindow::changeLed3Light);
    // ���Scene
    connect(ui->closeCamBtn, &QPushButton::clicked,this, &MainWindow::clearscene);
    // �������
    connect(ui->reInitCamBtn, &QPushButton::clicked, this, &MainWindow::openCam);

    /* ��ʱ���ڣ��ı���ʽ��
    connect(ui->cycleLd, &QLineEdit::inputRejected, this, &MainWindow::onTimeCycleTextFormat);
    connect(ui->cycleLd, &QLineEdit::textEdited, this, &MainWindow::onTimeCycleTextFormat);*/
}

// �������
void MainWindow::openCam() {
    ui->reInitCamBtn->setEnabled(false);
    ui->closeCamBtn->setEnabled(true);
    videothread* th1 = new videothread(1, 1);//���������ָ�룬�����캯��ִ������Ժ�ͻ�Ѿֲ�����ɾ��
    videothread* th2 = new videothread(2, 0);
    th1->start();
    th2->start();
}
// ���Scene
void MainWindow::clearscene() {
    scene_1->clear();
    scene_2->clear();
    //ui->reInitCamBtn->setEnabled(true);
    //ui->closeCamBtn->setEnabled(false);
}

// �л��ƹ�����
void MainWindow::changeLed1Light() {
    QString command = "x" + ui->pwmCbx->currentText();
    this->sendDataProcess(command);
}

void MainWindow::changeLed2Light() {
    QString command = "y" + ui->pwmCbx->currentText();
    this->sendDataProcess(command);
}

void MainWindow::changeLed3Light() {
    QString command = "z" + ui->pwmCbx->currentText();
    this->sendDataProcess(command);
}

// Ĭ�ϲ���ģʽ
void MainWindow::defParaMode() {
    defParaState = !defParaState;
    ui->pwmCbx->setEnabled(!defParaState);
    ui->timeCbx->setEnabled(!defParaState);
}
// �˶�ģʽ�л�����
void MainWindow::turnMode() {
    if (gamepadLinkState == true) {
        if (ui->inplaceRbtn->isChecked()) {
            modestate = inplace;
            ui->modeTxd->setText("ԭ��");
        }
        else if (ui->goforwardRbtn->isChecked()) {
            modestate = goforward;
            ui->modeTxd->setText("ǰ��");
        }
        else {
            modestate = retreat;
            ui->modeTxd->setText("����");
        }
    }
}

// ָ����Ʒ���ռ�ձȺ�ͨ��ʱ��
void MainWindow::goForward() {
    QString command;
    if (defParaState) {
        command = "f50";
    }
    else {
        command = "f" + ui->pwmCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::retreatMode() {
    QString command;
    if (defParaState) {
        command = "b50";
    }
    else {
        command = "b" + ui->pwmCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::stop() {
    QString command = "s";
    this->sendDataProcess(command);
}

void MainWindow::upliftHead() {
    QString command;
    if (defParaState) {
        command = "u50100";
    }
    else {
        command = "u" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::upliftTail() {
    QString command;
    if (defParaState) {
        command = "p50100";
    }
    else {
        command = "p" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::bowHead() {
    QString command;
    if (defParaState) {
        command = "d50100";
    }
    else {
        command = "d" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::bowTail() {
    QString command;
    if (defParaState) {
        command = "t50100";
    }
    else {
        command = "t" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::bowHead_2() {
    QString command;
    if (defParaState) {
        command = "j50100";
    }
    else {
        command = "j" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::bowTail_2() {
    QString command;
    if (defParaState) {
        command = "g50100";
    }
    else {
        command = "g" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
    }
    this->sendDataProcess(command);
}

void MainWindow::turnLeft() {
    QString command;
    switch (modestate) {
    case inplace:
        if (defParaState) {
            command = "l50200";
        }
        else {
            command = "l" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    case goforward:
        if (defParaState) {
            command = "n50020";
        }
        else {
            command = "n" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    case retreat:
        if (defParaState) {
            command = "m20050";
        }
        else {
            command = "m" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    }
    this->sendDataProcess(command);
}

void MainWindow::turnRight() {
    QString command;
    switch (modestate) {
    case inplace:
        if (defParaState) {
            command = "r50200";
        }
        else {
            command = "r" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    case goforward:
        if (defParaState) {
            command = "n20050";
        }
        else {
            command = "n" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    case retreat:
        if (defParaState) {
            command = "m50020";
        }
        else {
            command = "m" + ui->pwmCbx->currentText() + ui->timeCbx->currentText();
        }
        break;
    }
    this->sendDataProcess(command);
}

// ����Ҫ���͵�����
void MainWindow::sendDataProcess(QString command = "")
{
    QString strHex, data;
    if (ui->sendFormatChk->isChecked()) // 16����
    {
        // ���̶�ָ��
        if (command.isEmpty()) {
            strHex = ui->sendTxd->toPlainText();
        }
        else {
            strHex = command;
        }
        // ת���ı�
        strHex.remove(QRegExp("\\s"));  // ɾ�����пո�
        if (strHex.size() % 2 != 0)     // ����ɾ�����һ���ַ�
        {
            strHex = strHex.left(strHex.size() - 1);
        }

        // �Ƿ�ӻ��з�
        if (ui->newLineChk->isChecked())
        {
            strHex += "0D 0A";
        }

        QByteArray text = QByteArray::fromHex(strHex.toUtf8());
        // ��������
        m_pSerial->write(text);
        m_pSerial->waitForBytesWritten();

        // ��¼����������
        sendByte += text.size();
        ui->sendByteLbl->setText("Send:" + QString::number(sendByte));
    }
    else
    {
        if (command.isEmpty()) {
            data = ui->sendTxd->toPlainText();
        }
        else {
            data = command;
        }
        data.replace("\n", "\r\n");  // ��\n����\r\n

        // �Ƿ�ӻ��з�
        if (ui->newLineChk->isChecked())
        {
            data += "\r\n";
        }
        // ��������
        m_pSerial->write(data.toLocal8Bit());
        m_pSerial->waitForBytesWritten();

        // ��¼����������
        sendByte += data.toLocal8Bit().size();
        ui->sendByteLbl->setText("Send:" + QString::number(sendByte));
    }
}

void MainWindow::disconnectPort()
{
    ui->portNumCbx->setEnabled(true);
    ui->baudCbx->setEnabled(true);
    //ui->dataBitCbx->setEnabled(true);
    //ui->stopBitCbx->setEnabled(true);
    //ui->parityBitCbx->setEnabled(true);
    linkState = false;
    m_pSerial->close();
    ui->serialStateBtn->setText("�򿪴���");
}

/*//��ʾ��ǰʱ��
void MainWindow::updateLocalTime()
{
    ui->localTimeLbl->setText("��ǰʱ��:" + QTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}*/

// ���������жϿ�����
void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    //QSerialPort::ResourceError
    qDebug() << error;
    if (error == QSerialPort::PermissionError) {
        disconnectPort();
    }
}

// ���˿��Ƿ�Ͽ�
void MainWindow::checkTimeout()
{
    // ��ȡ���ö˿�
    QList<QSerialPortInfo> portInfo = QSerialPortInfo::availablePorts();
    QList<QString> portNames;
    // ��������Ķ˿�
    foreach(QSerialPortInfo info, portInfo)
    {
        portNames.append(info.portName());
        // �����ظ���
        if (-1 == ui->portNumCbx->findText(info.portName()))
        {
            ui->portNumCbx->addItem(info.portName());
            m_portNames.append(info.portName());
        }
    }
    // �Ƴ��Ѳ����ڵĶ˿�
    foreach(QString portName, m_portNames)
    {
        if (!portNames.contains(portName))
        {
            m_portNames.removeOne(portName);
            ui->portNumCbx->removeItem(ui->portNumCbx->findText(portName));
        }
    }
}

// �򿪶˿�
void MainWindow::onOpenPort()
{
    if (linkState == false)
    {
        // ���ö˿ں�
        m_pSerial->setPort(QSerialPortInfo(ui->portNumCbx->currentText()));
        // ���ò�����
        m_pSerial->setBaudRate(ui->baudCbx->currentText().toInt());
        // ��������λ
        m_pSerial->setDataBits(QSerialPort::Data8);
        m_pSerial->setStopBits(QSerialPort::OneStop);
        m_pSerial->setParity(QSerialPort::NoParity);
        /*switch (ui->dataBitCbx->currentIndex())
        {
        default:
        case 0:
            m_pSerial->setDataBits(QSerialPort::Data8);
            break;
        case 1:
            m_pSerial->setDataBits(QSerialPort::Data7);
            break;
        case 2:
            m_pSerial->setDataBits(QSerialPort::Data6);
            break;
        case 3:
            m_pSerial->setDataBits(QSerialPort::Data5);
            break;
        }
        // ����ֹͣλ
        switch (ui->stopBitCbx->currentIndex())
        {
        default:
        case 0:
            m_pSerial->setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            m_pSerial->setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 2:
            m_pSerial->setStopBits(QSerialPort::TwoStop);
            break;
        }
        // ����У��λ
        switch (ui->parityBitCbx->currentIndex())
        {
            // ��У��
        default:
        case 0:
            m_pSerial->setParity(QSerialPort::NoParity);
            break;
            // ��У��
        case 1:
            m_pSerial->setParity(QSerialPort::OddParity);
            break;
            //żУ��
        case 2:
            m_pSerial->setParity(QSerialPort::EvenParity);
            break;
        }*/
        if (!m_pSerial->open(QIODevice::ReadWrite))
        {
            //qDebug()<<m_pSerial->error();
            //QMessageBox::critical(this, "������ʾ", "���ڴ�ʧ�ܣ�����\n���ڱ�ռ�û�����������", "ȷ��");
            QMessageBox msgbox(this);
            msgbox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Dialog);
            msgbox.setWindowTitle("����");
            msgbox.setIcon(QMessageBox::Critical);
            msgbox.setText("���ڴ�ʧ�ܣ�����\n���ڱ�ռ�û�����������");
            msgbox.addButton("ȷ��", QMessageBox::AcceptRole);
            msgbox.setStyleSheet("QMessageBox {background-color: rgb(240,240,240);}QMessageBox QLabel#qt_msgbox_label{min-width: 180px;min-height: 40px;background-color:transparent;}");
            msgbox.exec();
            return;
        }
        ui->portNumCbx->setEnabled(false);
        ui->baudCbx->setEnabled(false);
        //ui->dataBitCbx->setEnabled(false);
        //ui->stopBitCbx->setEnabled(false);
        //ui->parityBitCbx->setEnabled(false);
        connect(m_pSerial, &QSerialPort::readyRead, this, &MainWindow::onReadData);
        ui->serialStateBtn->setText("�رմ���");
        linkState = true;
    }
    else
    {
        disconnectPort();
    }
}

// ��������
void MainWindow::onSendData()
{
    // ������
    if (linkState == false)
    {
        //QMessageBox::critical(this, "������ʾ", "����û�д򿪣�����\n��򿪴���", "ȷ��");
        QMessageBox msgbox(this);
        msgbox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Dialog);
        msgbox.setWindowTitle("����");
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("����û�д򿪣�����\n��򿪴���");
        msgbox.addButton("ȷ��", QMessageBox::AcceptRole);
        msgbox.setStyleSheet("QMessageBox {background-color: rgb(240,240,240);}QMessageBox QLabel#qt_msgbox_label{min-width: 180px;min-height: 40px;background-color:transparent;}");
        msgbox.exec();
        return;
    }
    this->sendDataProcess();
}

// ��������
void MainWindow::onReadData()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss\n");
    QByteArray msg = m_pSerial->readAll();
    // ��¼����������
    receiveByte += msg.size();
    ui->receiveByteLbl->setText("Receive:" + QString::number(receiveByte));
    msg += currentTime.toUtf8();
    // ֹͣ��ʾ
    if (ui->displayStateBtn->isChecked())
        return;

    // ������д���ı������� msg�����Ǳ��ر���(ANSI) ͨ��fromLocal8Bit()�����Unicode(UTF-16)
    QString textMsg(QString::fromLocal8Bit(msg));
    //qDebug()<<textMsg;
    m_textData.append(textMsg);

    // ������д��ʮ�����ƻ�����
    QString hexMsg;
    //QDataStream out(&msg, QIODevice::ReadWrite);
    //while (!out.atEnd())
    //{
    //    qint8 ch;
    //    out >> ch;
    //    hexMsg += QString("%1 ").arg(ch & 0xff, 2, 16, QLatin1Char('0'));
    //}
    //hexMsg = hexMsg.toUpper();// ת�ɴ�д
    hexMsg = msg.toHex(' ').toUpper();
    hexMsg.append(' ');
    m_hexData.append(hexMsg);

    // ������ƶ����ı�ĩβ���ٲ�������
    //QTextCursor cursor = ui->receiveTextBrowser->textCursor();
    //cursor.movePosition(QTextCursor::End);
    //ui->receiveTextBrowser->setTextCursor(cursor);
    ui->receiveTxb->moveCursor(QTextCursor::End);

    // ʮ��������ʾ
    if (ui->receiveFormatChk->isChecked())
    {
        ui->receiveTxb->insertPlainText(hexMsg);
    }
    else//�ı���ʾ
    {
        //ת�벢��ʾ
        //ui->receiveTextBrowser->append(textMsg);  // ���Զ��ӻ��з�
        ui->receiveTxb->insertPlainText(textMsg);
    }

    // ������ƶ����ı�ĩβ
    ui->receiveTxb->moveCursor(QTextCursor::End);
}

/*// ��ʱ��������
void MainWindow::onTimedData(bool checked)
{
    if (checked)
    {
        int msec = ui->cycleLd->text().toInt();
        m_timedSend.start(msec);
        ui->cycleLd->setEnabled(false);
    }
    else
    {
        m_timedSend.stop();
        ui->cycleLd->setEnabled(true);
    }
}*/

// ��������ʽ
/*void MainWindow::onReceiveAreaStyle(bool checked)
{
    // �Ƿ��ǰ׵׺���
    if (checked == true)
    {
        ui->receiveTxb->setStyleSheet("background-color: rgb(255, 255, 255);"
            "color: rgb(0, 0, 0);");
    }
    else
    {
        ui->receiveTxb->setStyleSheet("background-color: rgb(0, 0, 0);"
            "color: rgb(0, 255, 0);");
    }
}*/

// ���Ľ������ݸ�ʽ
void MainWindow::onChangeReceivedDataFormat(bool checked)
{
    // ���ʹ���ص���ʼ��
    ui->receiveTxb->clear();
    // ʮ��������ʾ
    if (checked)
    {
        ui->receiveTxb->insertPlainText(m_hexData);
    }
    else// �ı���ʾ
    {
        // ת�벢��ʾ
        ui->receiveTxb->insertPlainText(m_textData);
    }
}

// ��շ�������
void MainWindow::onClearSendData()
{
    sendByte = 0;
    ui->sendByteLbl->setText("Send:" + QString::number(sendByte));
    ui->sendTxd->clear();
}

// ��ս�������
void MainWindow::onClearReceivedData()
{
    receiveByte = 0;
    ui->receiveByteLbl->setText("Receive:" + QString::number(receiveByte));
    ui->receiveTxb->clear();
    m_textData.clear();
    m_hexData.clear();
}

// ֹͣ��ʾ
void MainWindow::onStopDisplay(bool checked)
{
    if (checked)
    {
        ui->displayStateBtn->setText("������ʾ");
    }
    else
    {
        ui->displayStateBtn->setText("ֹͣ��ʾ");
    }
}

// ��ʱ���ڸ�ʽ
/*void MainWindow::onTimeCycleTextFormat()
{
    if (ui->cycleLd->hasAcceptableInput() == false)
    {
        //QMessageBox::warning(this, "��ʾ", "�����ʽ����ȷ��\n���������֣�1-1000000��", "ȷ��");
        QMessageBox msgbox(this);
        msgbox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::Dialog );
        msgbox.setWindowTitle("��ʾ");
        msgbox.setIcon(QMessageBox::Warning);
        msgbox.setText("�����ʽ����ȷ��\n���������֣�1-1000000��");
        msgbox.addButton("ȷ��", QMessageBox::AcceptRole);
        msgbox.setStyleSheet("QMessageBox {background-color: rgb(240,240,240);}QMessageBox QLabel#qt_msgbox_label{min-width: 180px;min-height: 40px;background-color:transparent;}");
        msgbox.exec();
        ui->cycleLd->setText("1000");
    }
}*/

// �޸ķ���������
void MainWindow::modifySendDataFormatSlot()
{
    if (ui->sendFormatChk->isChecked())
    {
        // ת��16����
        QString strText = ui->sendTxd->toPlainText().replace('\n', "\r\n");
        QByteArray hex = strText.toLocal8Bit().toHex(' ').toUpper();
        ui->sendTxd->setPlainText(hex);
    }
    else
    {
        // ת���ı�
        QString strHex = ui->sendTxd->toPlainText();
        strHex.remove(QRegExp("\\s"));  // ɾ�����пո�
        if (strHex.size() % 2 != 0)     // ����ɾ�����һ���ַ�
        {
            strHex = strHex.left(strHex.size() - 1);
        }
        QByteArray text = QByteArray::fromHex(strHex.toUtf8());
        ui->sendTxd->setPlainText(QString::fromLocal8Bit(text));
    }
}

void MainWindow::sendDataChangeSlot()
{
    if (ui->sendFormatChk->isChecked())
    {
        QString data = ui->sendTxd->toPlainText();
        if (data.isEmpty())
            return;

        QRegExp rx("[0-9a-fA-F ]+$");
        if (!rx.exactMatch(data))
        {
            ui->sendTxd->textCursor().deletePreviousChar();  //ɾ�����ǰ���ַ�
            QMessageBox msgbox(this);
            msgbox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Dialog);
            msgbox.setWindowTitle("��ʾ");
            msgbox.setText("��������ȷ��ʽ0-9a-fA-F ���� 01 0a 0b");
            msgbox.addButton("ȷ��", QMessageBox::AcceptRole);
            msgbox.setStyleSheet("QMessageBox {background-color: rgb(240,240,240);}QMessageBox QLabel#qt_msgbox_label{min-width: 310px;min-height: 40px;background-color:transparent;}");
            msgbox.exec();
        }
    }
}

void MainWindow::updataPortNum(void) {

    //������ں�
    ui->portNumCbx->clear();

    //���� QSerialPortInfo, ��ӵ�������������
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()) {
        ui->portNumCbx->addItem(info.portName());
    }
}

void MainWindow::transformCurrentBtn(bool pressed, QString name, int index)
{
    if (pressed)
    {
        switch (index) {
            //buttonSelect���л��˶�ģʽ
        case 1:
            ui->currentBtn->setText(name);
            if (modestate == inplace) {
                modestate = goforward;
                ui->modeTxd->setText("ǰ��");
                ui->goforwardRbtn->setChecked(true);
            }
            else if (modestate == goforward) {
                modestate = retreat;
                ui->modeTxd->setText("����");
                ui->retreatRbtn->setChecked(true);
            }
            else {
                modestate = inplace;
                ui->modeTxd->setText("ԭ��");
                ui->inplaceRbtn->setChecked(true);
            }
            break;
            //buttonStart,�л�Ĭ�ϲ���ģʽ
        case 2:
            ui->currentBtn->setText(name);
            defParaMode();
            ui->defParaChk->setChecked(defParaState);
            break;
            //buttonA,��ָ��
        case 3:
            ui->currentBtn->setText(name);
            ui->bowHeadBtn_1->clicked();
            break;
            //buttonB
        case 4:
            ui->currentBtn->setText(name);
            ui->bowTailBtn_1->clicked();
            break;
            //buttonX
        case 5:
            ui->currentBtn->setText(name);
            ui->bowHeadBtn_3->clicked();
            break;
            //buttonY
        case 6:
            ui->currentBtn->setText(name);
            ui->bowTailBtn_3->clicked();
            break;
            //buttonL1����ָ��
        case 7:
            ui->currentBtn->setText(name);
            ui->upliftHeadBtn->clicked();
            break;
            //buttonR1
        case 8:
            ui->currentBtn->setText(name);
            ui->upliftTailBtn->clicked();
            break;
            //buttonL2
        case 9:
            ui->currentBtn->setText(name);
            break;
            //buttonR2
        case 10:
            ui->currentBtn->setText(name);
            break;
            //buttonL3
        case 11:
            ui->currentBtn->setText(name);
            //ui->stopBtn->clicked();
            break;
            //buttonR3
        case 12:
            ui->currentBtn->setText(name);
            break;
            //buttonUp,ֱ���˶�ָ��
        case 13:
            ui->currentBtn->setText(name);
            ui->goforwardBtn->clicked();
            break;
            //buttonDown
        case 14:
            ui->currentBtn->setText(name);
            ui->retreatBtn->clicked();
            break;
            //buttonLeft��ת��ָ��
        case 15:
            ui->currentBtn->setText(name);
            ui->turnLeftBtn->clicked();
            break;
            //buttonRight
        case 16:
            ui->currentBtn->setText(name);
            ui->turnRightBtn->clicked();
            break;
        default:
            break;
        }
    }
    else {
        ui->currentBtn->clear();
    }
}

void MainWindow::transformBtnVaule(int btnName, double value)
{
    switch (btnName) {
        //buttonL2,ֹͣ����
    case 1:
        if (value == 1)
        {
          ui->stopBtn->clicked();
        }
        break;
        //buttoR2,����LED����
    case 2:
        /*if (value == 1)
        {
            switch (modestate) {
            case inplace:
                ui->ledBtn_1->clicked();
                break;
            case goforward:
                ui->ledBtn_2->clicked();
                break;
            case retreat:
                ui->ledBtn_3->clicked();
                break;
            }
        }*/
        break;
        //buttonL3, axisLeftX
    case 3:
        if (defParaState)
            return;
        if (value == 1)
        {
            int pwmindex = ui->pwmCbx->currentIndex();
            if (pwmindex < (ui->pwmCbx->count() - 1)) {
                ++pwmindex;
                ui->pwmCbx->setCurrentIndex(pwmindex);
            }
            else {
                ui->pwmCbx->setCurrentIndex(0);
            }
        }
        else if (value == -1)
        {
            int pwmindex = ui->pwmCbx->currentIndex();
            if (pwmindex > 0) {
                --pwmindex;
                ui->pwmCbx->setCurrentIndex(pwmindex);
            }
            else {
                int max = ui->pwmCbx->count() - 1;
                ui->pwmCbx->setCurrentIndex(max);
            }
        }
        break;
        //buttonL3, axisLeftY
    case 4:
        if (defParaState)
            return;
        if (value == 1)
        {
            int pwmindex = ui->pwmCbx->currentIndex();
            if (pwmindex < (ui->pwmCbx->count() - 1)) {
                ++pwmindex;
                ui->pwmCbx->setCurrentIndex(pwmindex);
            }
            else {
                ui->pwmCbx->setCurrentIndex(0);
            }
        }
        else if (value == -1)
        {
            int pwmindex = ui->pwmCbx->currentIndex();
            if (pwmindex > 0) {
                --pwmindex;
                ui->pwmCbx->setCurrentIndex(pwmindex);
            }
            else {
                int max = ui->pwmCbx->count() - 1;
                ui->pwmCbx->setCurrentIndex(max);
            }
        }
        break;
        //buttonR3, axisRightX
    case 5:
        if (defParaState)
            return;
        if (value == 1)
        {
            int timeindex = ui->timeCbx->currentIndex();
            if (timeindex < (ui->timeCbx->count() - 1)) {
                ++timeindex;
                ui->timeCbx->setCurrentIndex(timeindex);
            }
            else {
                ui->timeCbx->setCurrentIndex(0);
            }
        }
        else if (value == -1)
        {
            int timeindex = ui->timeCbx->currentIndex();
            if (timeindex > 0) {
                --timeindex;
                ui->timeCbx->setCurrentIndex(timeindex);
            }
            else {
                int max = ui->timeCbx->count() - 1;
                ui->timeCbx->setCurrentIndex(max);
            }
        };
        break;
        //buttonR3, axisRightY
    case 6:
        if (defParaState)
            return;
        if (value == 1)
        {
            int timeindex = ui->timeCbx->currentIndex();
            if (timeindex < (ui->timeCbx->count() - 1)) {
                ++timeindex;
                ui->timeCbx->setCurrentIndex(timeindex);
            }
            else {
                ui->timeCbx->setCurrentIndex(0);
            }
        }
        else if (value == -1)
        {
            int timeindex = ui->timeCbx->currentIndex();
            if (timeindex > 0) {
                --timeindex;
                ui->timeCbx->setCurrentIndex(timeindex);
            }
            else {
                int max = ui->timeCbx->count() - 1;
                ui->timeCbx->setCurrentIndex(max);
            }
        }
        break;
    default:
        break;
    }
}
// �����غ���
QImage cvMat2QImage(const cv::Mat& mat)//opencv��Mat��������ת��Ϊqt��QImage��������
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar* pSrc = mat.data;
        for (int row = 0; row < mat.rows; row++)
        {
            uchar* pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        pSrc = 0;
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar* pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        pSrc = 0;
        return image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar* pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        pSrc = 0;
        return image.copy();
    }
    else
    {
        return QImage();
    }
}

void MainWindow::on_rstpushbutton_clicked()
{
    reset = !reset;
}

void MainWindow::on_flippushbutton_clicked()
{
    emit signal_flip_1();
    emit signal_flip_2();
}

void MainWindow::on_openpushbutton_clicked()
{
    emit signal_openrecord_1();
    emit signal_openrecord_2();
    ui->openpushbutton->setEnabled(false);
    ui->closepushbutton->setEnabled(true);

    //������ɫ�������
}

void MainWindow::on_closepushbutton_clicked()
{
    emit signal_endrecord_1();
    emit signal_endrecord_2();
    ui->openpushbutton->setEnabled(true);
    ui->closepushbutton->setEnabled(false);
}


void MainWindow::show_image_1(QImage dstImage)
{
    dstImage = dstImage.scaled(600, 340, Qt::KeepAspectRatio, Qt::SmoothTransformation);//����16:9  ����������
    scene_1->clear();   //���scene_1�����ݣ�����addPixmap�ᵼ��scene_1ռ�õ��ڴ�Խ��Խ��
    scene_1->addPixmap(QPixmap::fromImage(dstImage));//QImage->QPixmap
    if (!reset) {
        ui->graphicsView->setScene(scene_1);
        ui->graphicsView->resize(scene_1->width() + 3, scene_1->height() + 2);//������С����
        ui->graphicsView->show();
    }
    else {
        ui->graphicsView_2->setScene(scene_1);
        ui->graphicsView_2->resize(scene_1->width() + 3, scene_1->height() + 2);//������С����
        ui->graphicsView_2->show();
    }
}


void MainWindow::show_image_2(QImage dstImage)
{
    dstImage = dstImage.scaled(600, 340, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scene_2->clear();
    scene_2->addPixmap(QPixmap::fromImage(dstImage));
    if (!reset) {
        ui->graphicsView_2->setScene(scene_2);
        ui->graphicsView_2->resize(scene_2->width() + 3, scene_2->height() + 2);
        ui->graphicsView_2->show();
    }
    else {
        ui->graphicsView->setScene(scene_2);
        ui->graphicsView->resize(scene_2->width() + 3, scene_2->height() + 2);
        ui->graphicsView->show();
    }
}

void videothread::flipmat()
{
    flip_flag = !flip_flag;
}


void videothread::openrecord()
{
    if (record_flag == false) {

        time_t timep;
        struct tm* p;
        char name[256] = { 0 };//��ϵͳʱ����ص��ļ���
        time(&timep);//��ȡ��1970������˶����룬����time_t���͵�timep
        p = localtime(&timep);//��localtime������ת��Ϊstruct tm�ṹ��
        sprintf(name, "%d_rec-%d-%d-%d %02d-%02d-%02d.avi", cam, 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

        writer = new VideoWriter;
        writer->open(name, VideoWriter::fourcc('M', 'P', '4', '2'), 15, Size(1280, 720), true);
        qDebug() << cam << QString::fromLocal8Bit("���ڿ�ʼ¼��");
        record_flag = true;
    }
}

void videothread::closeCam() {
    QMutexLocker locker(&m_mutex);// �˴�����,��ֹ���ʳ�ͻ
    m_closeflag = true;
}

void videothread::endrecord()
{
    if (record_flag == true) {
        record_flag = false;
        msleep(50);
        writer->release();
        delete writer;
        qDebug() << cam << QString::fromLocal8Bit("���ڽ���¼��");
    }
}


videothread::videothread(int cam, int order)
{
    this->cam = cam;
    this->order = order;
}


videothread::~videothread()
{
}

void videothread::run()//���߳�
{
    Camera frame;
    frame.setcam(cam);    //��ȡ����ͷ������
    frame.open_camera();	//������ͷ
    if (!frame.isopened())
    {
        frame.setcam(order);    //��ȡ����ͷ������
        frame.open_camera();
    }
    while (1)     //���߳̽�����ѭ�������ϵض�ȡͼ�񲢷����ź�
    {
        Mat srcImage = frame.read_frame();
        if (flip_flag == true) flip(srcImage, srcImage, -1);
        if (record_flag == true) writer->write(srcImage);
        QImage dstImage = cvMat2QImage(srcImage);
        //qDebug() << QString::fromLocal8Bit("����ͷ������") << cam << QString::fromLocal8Bit("�ֱ���") << srcImage.cols << srcImage.rows << QString::fromLocal8Bit("֡��") << frame.nFps;//�ֱ���1280*720
        emit signal_tomainthread(dstImage);
        msleep(33);//����ͷ֡��
        {
            QMutexLocker locker(&m_mutex);// �˴�����,��ֹ���ʳ�ͻ
            if (m_closeflag)//��ÿ��ѭ���ж��Ƿ�������У�������о��˳�ѭ��
            {
                return;
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
