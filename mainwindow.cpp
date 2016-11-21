#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"
#include "QMessageBox"
#include "QFileDialog"
#include "QTimer"
//#include "QProgressBar"
#include "QFile"
//#include "QWidget"



QTimer timer;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget::setWindowTitle("STM UART Downloader");  // Название программы вверху окна


   /* bar=new QProgressBar(this);
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->show();*/

    ui->progressBar->setMaximum(0);                  // установим минимальное значение для prigress bar
    ui->progressBar->setMaximum(100);                //  установим максимальное значение для prigress bar
    ui->progressBar->setVisible(false);              //  сделаем этот виджет невидимым ...пока.

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) // цикл записи доступных COM портов в виджет dropdown
    {
        ui->COMx->addItem(info.portName());
    }

    /*
    foreach (const qint32 baudRate, QSerialPortInfo::standardBaudRates()) {
        ui->comboBox->addItem(QString::number(baudRate));
    }
    */
    QStringList List;                               // создаем объект класса QStringList и записываем
    List<<QString::number(QSerialPort::Baud1200)    // доступные скорости для QSerialPort.
        <<QString::number(QSerialPort::Baud2400)
        <<QString::number(QSerialPort::Baud4800)
        <<QString::number(QSerialPort::Baud9600)
        <<QString::number(QSerialPort::Baud19200)
        <<QString::number(QSerialPort::Baud38400)
        <<QString::number(QSerialPort::Baud57600)
        <<QString::number(QSerialPort::Baud115200);

    ui->SPEED->addItems(List);                      // добавляем этот список в виджет выбора скорости порта
     ui->Disconnect->setEnabled(false);             // кнопку Disconnect и Load_to_STM делаем не активными
     ui->Load_to_STM->setEnabled(false);

    // QObject::connect(&serial,SIGNAL(bytesWritten(qint64)),this,SLOT(TransmiteBytes()));

     /*сигнал readyRead привязываем к объекту QSerialPort serial, слот RecieveBytes() к данному классу окна MainWindow*/
     QObject::connect(&serial,SIGNAL(readyRead()),this,SLOT(RecieveBytes()));
     /*сигнал timeout привязываем к объекту QTimer timer, слот TimerExpire() к данному классу окна MainWindow*/
     QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(TimerExpire()));

}
MainWindow::~MainWindow()
{
    delete ui;
    file.close();
    serial.close();
}



void MainWindow::on_Connect_clicked()
{
    serial.setPortName(ui->COMx->currentText());  // устанавливаеми имя порта с виджета выбора доступных COM в системе
    if(serial.open(QIODevice::ReadWrite))          // открываем данный порт с проверкой
       {                                         // если открыли успешно продолжаем насройки порта
        switch(ui->SPEED->currentIndex()){       // задаем скорость порта в зависимости от выбора в виджете SPEED
             case 0: serial.setBaudRate(QSerialPort::Baud1200); break;
             case 1: serial.setBaudRate(QSerialPort::Baud2400); break;
             case 2: serial.setBaudRate(QSerialPort::Baud4800); break;
             case 3: serial.setBaudRate(QSerialPort::Baud9600); break;
             case 4: serial.setBaudRate(QSerialPort::Baud19200); break;
             case 5: serial.setBaudRate(QSerialPort::Baud38400); break;
             case 6: serial.setBaudRate(QSerialPort::Baud57600); break;
             case 7: serial.setBaudRate(QSerialPort::Baud115200); break;
            }

        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        serial.setStopBits(QSerialPort::OneStop);


        /* делаем недоступными кнопки выюора порта и скорости и кнопки Connect */
        ui->COMx->setEnabled(false);
        ui->SPEED->setEnabled(false);
        ui->Connect->setEnabled(false);
        ui->Disconnect->setEnabled(true);   // Diconnect активируем
     }
    else        // если порт открылся с ошибкой значит занят или не рабочий.
       QMessageBox::information(this,tr("Error"),"COM port is busy!");      // Выводим QMessageBox




}

void MainWindow::on_Disconnect_clicked()
{
    serial.close();
    ui->COMx->setEnabled(true);
    ui->SPEED->setEnabled(true);
    ui->Connect->setEnabled(true);
    ui->Disconnect->setEnabled(false);

}

void MainWindow::on_File_clicked()          //при нажатии на кнопку File.. открываем диалог по выбору файла бинарника
{
    QString filename=QFileDialog::getOpenFileName(this,tr("Open file"),"D:\\STM32_проекты","All Files (*.*);;Binary files (*.bin);;Text files (*.txt)");

    /*file.setFileName(filename);
    file.open(QIODevice::ReadOnly);*/
    ui->FileName->setText(filename);

    if(file.exists(ui->FileName->text()))
      {
        if(filename.endsWith(".bin"))
            ui->Load_to_STM->setEnabled(true);
        else
          ui->Load_to_STM->setEnabled(false);
      }
    else
      ui->Load_to_STM->setEnabled(false);
}
void MainWindow::on_FileName_textEdited(const QString &arg1)
{
    if(file.exists(arg1))
      {
        if(arg1.endsWith(".bin"))
            ui->Load_to_STM->setEnabled(true);
      }
    else
      ui->Load_to_STM->setEnabled(false);
}


void MainWindow::on_Load_to_STM_clicked()
{
    QByteArray temp_array("hello st\r");

    ui->Load_to_STM->setEnabled(false);
    /*file.setFileName(ui->FileName->text());
    file.open(QIODevice::ReadOnly);
    temp_array=file.read(5000);
    //array=file.readAll();//
    //file.close();*/
    serial.write(temp_array);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    //timer.start(10000);

}
void MainWindow::TimerExpire()
{
    timer.stop();
    serial.close();
    ui->COMx->setEnabled(true);
    ui->SPEED->setEnabled(true);
    ui->Connect->setEnabled(true);
    ui->Load_to_STM->setEnabled(true);
    ui->Disconnect->setEnabled(false);

    QMessageBox::information(this,tr("Error"),"ERROR! STM32 not ready! ");
}

void MainWindow::RecieveBytes()
{
      QByteArray array_x=serial.readAll();
      array.append(array_x);

      if(array.endsWith("get size"))
         {

          array.clear();
          file.setFileName(ui->FileName->text());
          file.open(QIODevice::ReadOnly);
          quint32 size=file.size();
          QByteArray x=QByteArray::number(size);
          QByteArray y="\r";
          x.append(y);
          serial.write(x);
         }
        else if(array.endsWith("get data"))
         {
             qint32 a=(file.pos()*100)/file.size();
             ui->progressBar->setValue(a);
            a=(file.size())-(file.pos());

            if(a<10000)
             {
               QByteArray temp_array=file.read(a);
                serial.write(temp_array);
                file.close();

             }
             else
             {
               QByteArray temp_array=file.read(10000);
               serial.write(temp_array);

             }
         }
       else if(array.endsWith("crc ok!"))
         {
           ui->progressBar->setVisible(false);
           file.close();
           serial.close();
           ui->COMx->setEnabled(true);
           ui->SPEED->setEnabled(true);
           ui->Connect->setEnabled(true);
           ui->Load_to_STM->setEnabled(true);
           ui->Disconnect->setEnabled(false);
           QMessageBox::information(this,tr("Message"),"OK!");
         }
       else if(array.endsWith("crc error!"))
         {
            ui->progressBar->setVisible(false);
            file.close();
            serial.close();
            ui->COMx->setEnabled(true);
            ui->SPEED->setEnabled(true);
            ui->Connect->setEnabled(true);
            ui->Load_to_STM->setEnabled(true);
            ui->Disconnect->setEnabled(false);
            QMessageBox::information(this,tr("Message"),"CRC ERROR!");
         }

}

/*void MainWindow::SerialPortError()
{

     QMessageBox::information(this,tr("Message"),"OK!");
}*/
