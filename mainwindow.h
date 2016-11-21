#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "QFile"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"
//#include "QProgressBar"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_Connect_clicked();
    void on_Disconnect_clicked();
    void on_File_clicked();
    void on_Load_to_STM_clicked();
   // void TransmiteBytes();
    void RecieveBytes();
    void TimerExpire();
   // void SerialPortError();
    void on_FileName_textEdited(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QFile       file;
    QSerialPort serial;
    QByteArray  array;

};

#endif // MAINWINDOW_H
