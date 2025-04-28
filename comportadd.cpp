#include "comportadd.h"
#include "ui_comportadd.h"

COMportAdd::COMportAdd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::COMportAdd)
{
    ui->setupUi(this);
    loadPorts();
}

COMportAdd::~COMportAdd()
{
    delete ui;
    if (_serialPort != nullptr) {
        _serialPort->close();
        delete _serialPort;
    }
}

void COMportAdd::loadPorts(){
    foreach (auto &port, QSerialPortInfo::availablePorts()) {
        ui->cmbPorts->addItem(port.portName());
    }
}

void COMportAdd::on_btnOpenPort_clicked()
{
    if (_serialPort != nullptr) {
        _serialPort->close();
        delete _serialPort;
    }
    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(ui->cmbPorts->currentText());
    _serialPort->setBaudRate(QSerialPort::Baud9600);
    _serialPort->setDataBits(QSerialPort::Data8);
    _serialPort->setParity(QSerialPort::NoParity);
    _serialPort->setStopBits(QSerialPort::OneStop);
    if (_serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, "Result", "success");
        QObject::connect(_serialPort, &QSerialPort::readyRead, this, &COMportAdd::readData);
    } else {
        QMessageBox::critical(this, "Port Error", "unable to open port");
    }
}

void COMportAdd::readData(){
    if (!_serialPort->isOpen()){
        QMessageBox::critical(this, "port error", "port is not opened");
    }

    auto data = _serialPort->readAll();
    ui->lstMessages->addItem(QString(data));
}

void COMportAdd::on_btnStart_clicked()
{

}


void COMportAdd::on_btnStop_clicked()
{

}


void COMportAdd::on_btnChangeSpeed_clicked()
{

}

