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

/*
*   Конструктор для MainWindow
***************************************************/
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
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

/*
*   Деструктор для MainWindow
***************************************************/
MainWindow::~MainWindow()
{
    delete ui;
    file.close();
    serial.close();
}

/*
*   слот для сигнала clicked() нопки Connect
***************************************************/
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

/*
*   слот для сигнала clicked() нопки Disconnect
****************************************************/
void MainWindow::on_Disconnect_clicked()
{
    serial.close();                     // закрываем COM порт
    ui->COMx->setEnabled(true);         // виджет COMx активный
    ui->SPEED->setEnabled(true);        // виджет SPEED активный
    ui->Connect->setEnabled(true);      // кнопка Connect активна
    ui->Disconnect->setEnabled(false);  // кнопка Disconnect  не активна
}

/*
*   слот для сигнала clicked() нопки File...
***************************************************/
void MainWindow::on_File_clicked()
{
   /*при нажатии на кнопку File.. открываем диалог по выбору файла бинарника*/
    QString filename=QFileDialog::getOpenFileName(this,tr("Open file"),"C:\\","All Files (*.*);;Binary files (*.bin);;Text files (*.txt)");


    ui->FileName->setText(filename);             // записываем полный путь до бинарника в LineEdit

    if(file.exists(ui->FileName->text()))       //проверка наличия файла с полным именем в LineEdit
      {
        if(filename.endsWith(".bin"))           // елси существует проверяем на тип *.bin
            ui->Load_to_STM->setEnabled(true);  // есои открыли бинарник Load_to_STM делаем активным
        else
          ui->Load_to_STM->setEnabled(false);   // есои открыли не  бинарник Load_to_STM делаем  не активным
      }
    else
      ui->Load_to_STM->setEnabled(false);       // если файла с полным именем не существует Load_to_STM делаем не активной
}

/*
*   слот для сигнала textEdit виджета LineEdit
***************************************************/
void MainWindow::on_FileName_textEdited(const QString &arg1)
{   /* любое изменение строки с полным именем файла с виджета LineEdit проверяем */
    if(file.exists(arg1))
      {
        if(arg1.endsWith(".bin"))                   //  елси существует проверяем на тип *.bin
            ui->Load_to_STM->setEnabled(true);      // есои открыли бинарник Load_to_STM делаем активным
      }
    else
      ui->Load_to_STM->setEnabled(false);           // если файла с полным именем не существует Load_to_STM делаем не активной
}

/*
*   слот для сигнала clicked() нопки Load_to_STM
***************************************************/
void MainWindow::on_Load_to_STM_clicked()
{
    QByteArray temp_array("hello st\r");            // создаем временный объект QByteArray и записываем в него строчку "hello st\r"

    ui->Load_to_STM->setEnabled(false);             // делаем кнопку Load_to_STM не активной на время загрузки бинарника

    serial.write(temp_array);                       // записываем в порт массив из QByteArray для начала передачи с STM32
    ui->progressBar->setValue(0);                   // Для progressBar устанавливаем начальное  значение 0 %
    ui->progressBar->setVisible(true);              // progressBar делаем видимым для индикации процесса загрузки
    timer.start(2000);                              // запускаем таймер с периодом 2 сек
}

/*
*   слот для сигнала timeout() QTimer timer
*****************************************************/
void MainWindow::TimerExpire()
{
   /* если таймер истек значит ответа от STM32 нет 2 сек */
    timer.stop();                               // останавливаем таймер
    serial.close();                             // закрываем QSerial port
    ui->COMx->setEnabled(true);                 // виджет COMx активный
    ui->SPEED->setEnabled(true);                // виджет SPEED активный
    ui->Connect->setEnabled(true);              // кнопка Connect активна
    ui->Load_to_STM->setEnabled(true);          // кнопка Load_to_STM активна
    ui->Disconnect->setEnabled(false);          // кнопка Disconnect  не активна
    QMessageBox::information(this,tr("Error"),"ERROR! STM32 not ready! ");  // выводим сообщение об ошибки что STM32 не отвечает более 2 сек
}

/*
*   слот для сигнала redyRead() QSerialPort serial
******************************************************/
void MainWindow::RecieveBytes()
{
   /* каждый раз при получении ответа от STM32 или при подтверждении
      очередной переданной порции делаем рестарт таймера на следующие 2 секунды */
      timer.start(2000);
    /*временный массив  QByteArray array_x для считывания данных с COM порта каждый раз по сигналу readyRead() */
      QByteArray array_x=serial.readAll();
      array.append(array_x);                  // склеиваем части приходящих байт в одну строку -сообщение в  QByteArray array

      if(array.endsWith("get size"))        // сравниваем конец строки с сообщением "get size"
         {
          /* если пришел ответ от STM32 "get size"  готовимся послать размер бинарника в виде символьной строки */
          array.clear();                           // очищаем  QByteArray array для приема след. сообщениий
          file.setFileName(ui->FileName->text());  // задаем полное имя файла+путь с виджета LineEdit
          file.open(QIODevice::ReadOnly);          // открываем указанный файл с атрибутом  ReadOnly
          quint32 size=file.size();                // в переменную size записываем размер бинарника
          QByteArray x=QByteArray::number(size);   // в объект QByteArray x записываем размер файла в виде символьной строки (216436==32 31 36 34 33 36)
          QByteArray y="\r";                      // создаем объект QByteArray y в который записываем символ "\r"
          x.append(y);                            // склеиваем массивы x и y в x получаем 32 31 36 34 33 36 0D
          serial.write(x);                         // записываем в порт эту строку с размером файла
         }
        else if(array.endsWith("get data"))             /* если от МК приняли сообщение "get data"*/
         {

            qint32 a=(file.pos()*100)/file.size();      // в a заносим процент переданных частей от файла
             ui->progressBar->setValue(a);              // отображаем этот процент на progressbar
            a=(file.size())-(file.pos());               // в a заносим число с текущим положением точки считывание очередной порции байт
        /* если a<10000 значит остался небольшой "хвост" в конце файла который надо отправить порцией меньшей чем 10000 */
            if(a<10000)
             {
               QByteArray temp_array=file.read(a);      // считываем количество этх данных во временный массив QByteArray temp_array
               QByteArray y="\r";                      // создаем объект QByteArray y в который записываем символ "\r"
               temp_array.append(y);                   // склеиваем массивы temp_array и y в temp_array
               serial.write(temp_array);               // отправляем этот "хвост" в порт
               file.close();                          // закрываем открытый файл так как все данные с файла считали

             }
         /* иначе если a>10000 нужно считать из файла очередную порцию в 10000 байт и отправить их через порт */
            else
             {
               QByteArray temp_array=file.read(10000);  // читаем 10000 байт с файла во временный массив QByteArray temp_array
               QByteArray y="\r";                      // создаем объект QByteArray y в который записываем символ "\r"
               temp_array.append(y);                   // склеиваем массивы temp_array и y в temp_array
               serial.write(temp_array);                // записываем очередные 10000 байт в порт для отправки на МК

             }
         }
       else if(array.endsWith("crc ok!"))                /* если от МК приняли сообщение "crc ok!" */
         {
           timer.stop();                                // останавливаем таймер
           ui->progressBar->setVisible(false);          // progressbar делаем невидимым
           file.close();                                // закрываем открытый файл так как все данные с файла считали
           serial.close();                              // закрываем COM порт
           ui->COMx->setEnabled(true);                  // виджет COMx активный
           ui->SPEED->setEnabled(true);                 // виджет SPEED активный
           ui->Connect->setEnabled(true);               // кнопка Connect активна
           ui->Load_to_STM->setEnabled(true);           // кнопка Load_to_STM активна
           ui->Disconnect->setEnabled(false);           // кнопка Disconnect  не активна
           QMessageBox::information(this,tr("Message"),"OK!");  // выводим сообщение об успешной передачи
         }
       else if(array.endsWith("crc error!"))            /* если от МК приняли сообщение "crc error!" */
         {
            timer.stop();                               // останавливаем таймер
            ui->progressBar->setVisible(false);         // progressbar делаем невидимым
            file.close();                               // закрываем открытый файл так как все данные с файла считали
            serial.close();                             // закрываем COM порт
            ui->COMx->setEnabled(true);                 // виджет COMx активный
            ui->SPEED->setEnabled(true);                // виджет SPEED активный
            ui->Connect->setEnabled(true);              // кнопка Connect активна
            ui->Load_to_STM->setEnabled(true);          // кнопка Load_to_STM активна
            ui->Disconnect->setEnabled(false);          // кнопка Disconnect  не активна
            QMessageBox::information(this,tr("Message"),"CRC ERROR!");  // выводим сообщение об ошибки контрольной суммы
         }

}//dfkk;hjfgcxkmcfh;lkmfghlkmgxfmlgckx

