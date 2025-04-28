#ifndef COMPORTADD_H
#define COMPORTADD_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

namespace Ui {
class COMportAdd;
}

class COMportAdd : public QDialog
{
    Q_OBJECT

public:
    explicit COMportAdd(QWidget *parent = nullptr);
    ~COMportAdd();


    void readData();

private slots:
    void on_btnOpenPort_clicked();

    void on_btnStart_clicked();

    void on_btnStop_clicked();

    void on_btnChangeSpeed_clicked();

private:
    Ui::COMportAdd *ui;
    QSerialPort *_serialPort;

    void loadPorts();
};

#endif // COMPORTADD_H
