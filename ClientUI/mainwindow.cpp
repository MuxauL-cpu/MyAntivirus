#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Operations.h>
#include "IPC.h"
#include <thread>
#include <QThread>
#include <QTimer>
#include <locale.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	std::thread Thread1(&MainWindow::connectPipe,this);
	tmr = new QTimer();
	tmr->setInterval(1000);
	ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	ui->dateTimeEdit->setMinimumDateTime(QDateTime::currentDateTime());
	SC_HANDLE sc = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
#ifndef NDEBUG
	SC_HANDLE os = OpenService(sc, L"ClientUIDbg", SERVICE_START);
#else
	SC_HANDLE os = OpenService(sc, L"ClientUI", SERVICE_START);
#endif
	StartService(os, 0, NULL);

	CloseServiceHandle(sc);
	CloseServiceHandle(os);
	
	Thread1.detach();

	connect(this, &MainWindow::output, ui->resultTextEdit, &QTextEdit::insertPlainText);
	connect(this, &MainWindow::writeText, ui->resultTextEdit, &QTextEdit::setText);
	connect(this, &MainWindow::outputScheduled, ui->resultTextEdit_2, &QTextEdit::insertPlainText);
	connect(this, &MainWindow::writeTextScheduled, ui->resultTextEdit_2, &QTextEdit::setText);
	connect(this, &MainWindow::outputMonitor, ui->resultTextEdit_3, &QTextEdit::insertPlainText);
	connect(this, &MainWindow::writeTextMonitor, ui->resultTextEdit_3, &QTextEdit::setText);
	connect(this, &MainWindow::setScanStopButton, ui->stopButton, &QPushButton::setEnabled);
	connect(this, &MainWindow::setScanStartButton, ui->startScanButton, &QPushButton::setEnabled);
	connect(this, &MainWindow::setScheduleStopButton, ui->stopButton_2, &QPushButton::setEnabled);
	connect(this, &MainWindow::setScheduleSetButton, ui->setButton, &QPushButton::setEnabled);
	connect(this, &MainWindow::setScheduleCancelButton, ui->cancelSchedule, &QPushButton::setEnabled);
	connect(this, &MainWindow::setStartMonitoringButton, ui->monitorButton, &QPushButton::setEnabled);
	connect(this, &MainWindow::setCancelMonitoringButton, ui->cancelMonitorButton, &QPushButton::setEnabled);
	connect(tmr, SIGNAL(timeout()), this, SLOT(updateTime()));
	connect(this, &MainWindow::logAppend, ui->logTextEdit, &QTextEdit::append);
	tmr->start();

}
MainWindow::~MainWindow()
{
	StopMonitoring = true;
	Sleep(1000);
    DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	DisconnectNamedPipe(hPipeScheduled);
	CloseHandle(hPipeScheduled);
	DisconnectNamedPipe(hPipeMonitor);
	CloseHandle(hPipeMonitor);
    delete ui;
}

void MainWindow::connectPipe()
{
	hPipe = CreateFile(lpszPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	while (hPipe == INVALID_HANDLE_VALUE)
	{
		hPipe = CreateFile(lpszPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		Sleep(1);
	}
	hPipeScheduled = CreateFile(lpszScPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	while (hPipeScheduled == INVALID_HANDLE_VALUE)
	{
		hPipeScheduled = CreateFile(lpszScPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		Sleep(1);
	}
	hPipeMonitor = CreateFile(lpszPipeMonitorName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	while (hPipeMonitor == INVALID_HANDLE_VALUE)
	{
		hPipeMonitor = CreateFile(lpszPipeMonitorName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		Sleep(1);
	}
}

void MainWindow::startMonitoring()
{
	Writeint8_t(hPipeMonitor, Operation::MONITOR);
	std::u16string toSend = ui->pathLineEdit_3->text().toStdU16String();
	WriteU16String(hPipeMonitor, toSend);
	OperationResult oper = (OperationResult)Readint8_t(hPipeMonitor);
	OperationResult previousOper = OperationResult::FAILED;
	setCancelMonitoringButton(true);
	while (true)
	{
		Writeint8_t(hPipeMonitor, Operation::GET_STATE);
		oper = (OperationResult)Readint8_t(hPipeMonitor);
		Sleep(10);
		if (oper != OperationResult::SUCCESS)
		{

			if (oper == OperationResult::MONITORING)
			{
				if (previousOper != oper)
				{
					previousOper = oper;
					setCancelMonitoringButton(true);
				}
				if (StopMonitoring)
				{
					Writeint8_t(hPipeMonitor, Operation::STOP);
					ui->monitorStatusLabel->setText("");
					StopMonitoring = false;
					setStartMonitoringButton(true);
					return;
				}
				Writeint8_t(hPipeMonitor, Operation::GET_STATE);
				oper = (OperationResult)Readint8_t(hPipeMonitor);
			}
			else if (oper == OperationResult::RUNNING)
			{
				if (previousOper != oper)
				{
					previousOper = oper;
					setCancelMonitoringButton(false);
				}
			}
		}
		else if (oper == OperationResult::SUCCESS)
		{
			writeTextMonitor("");
			Writeint8_t(hPipeMonitor, Operation::GET_STATISTICS);
			int numofParts = Readuint32_t(hPipeMonitor);
			for (int i = 0; i < numofParts; i++)
			{
				std::u16string stat = ReadU16String(hPipeMonitor);
				if (stat != u"")
					outputMonitor(QDir::fromNativeSeparators(QString::fromStdU16String(stat)));
			}
		}
		Sleep(1);
	}
}

void MainWindow::startScheduling()
{
	QTime qtime(ui->dateTimeEdit->time().hour(), ui->dateTimeEdit->time().minute(), 0, 0);
	QDateTime qDateTime(ui->dateTimeEdit->date(), qtime);
	qint64 secondsSinceEpoch = qDateTime.toSecsSinceEpoch();
	Writeint8_t(hPipeScheduled, Operation::SCANSCHEDULED);
	Writeint64_t(hPipeScheduled, secondsSinceEpoch);
	std::u16string Send = ui->pathLineEdit_2->text().toStdU16String();
	WriteU16String(hPipeScheduled, Send);
	OperationResult operation = (OperationResult)Readint8_t(hPipeScheduled);
	OperationResult previousStatus = OperationResult::SCHEDULED;
	if (operation != OperationResult::SUCCESS)
	{
		Sleep(300);
		while (operation != OperationResult::SUCCESS)
		{
			Writeint8_t(hPipeScheduled, Operation::GET_STATE);
			operation = (OperationResult)Readint8_t(hPipeScheduled);
			if (operation == OperationResult::RUNNING)
			{
				if (StopScheduleScan)
				{
					Writeint8_t(hPipeScheduled, Operation::STOP);
					Sleep(500);
					break;
				}
				if (previousStatus != operation)
				{
					previousStatus = operation;
					setScheduleStopButton(true);
					QDateTime currTime(QDateTime::currentDateTime());
					logAppend(currTime.time().toString() + ": Started scheduled scanning " + QString::fromStdU16String(Send));
					setScheduleCancelButton(false);
				}
			}
			else if (operation == OperationResult::SCHEDULED)
			{
				if (CancelScheduleTime)
				{
					Writeint8_t(hPipeScheduled, Operation::CANCELSCHEDULE);
					CancelScheduleTime = false;
					setScheduleCancelButton(false);
					QDateTime currTime(QDateTime::currentDateTime());
					logAppend(currTime.time().toString() + ": Schedule scan canceled");
					return;
				}
			}
			else if (operation == OperationResult::FAILED)
			{
				writeTextScheduled("");
				setScheduleSetButton(true);
				outputScheduled("Failed, wrong time");
				return;
			}
			Sleep(1000);
		}

		writeTextScheduled("");
		Writeint8_t(hPipeScheduled, Operation::GET_STATISTICS);
		int PartNum = Readuint32_t(hPipeScheduled);

		for (int i = 0; i < PartNum; i++)
		{
			std::u16string stat = ReadU16String(hPipeScheduled);
			if (stat != u"")
				outputScheduled(QDir::fromNativeSeparators(QString::fromStdU16String(stat)));
		}
		if (StopScheduleScan)
		{
			outputScheduled("Scan hasn't been finished");
			StopScheduleScan = false;
		}
		else
		{
			QDateTime currTime(QDateTime::currentDateTime());
			logAppend(currTime.time().toString() + ": Finished scheduled scanning " + QString::fromStdU16String(Send));
		}
		setScheduleSetButton(true);
		setScheduleCancelButton(false);
		setScheduleStopButton(false);
	}
}

void MainWindow::scan(bool scheduled)
{
	Writeint8_t(hPipe, Operation::SCANPATH);
	std::u16string Send = ui->pathLineEdit->text().toStdU16String();
	WriteU16String(hPipe, Send);
	OperationResult operation = (OperationResult)Readint8_t(hPipe);
	OperationResult prevOperation = operation;
	if (operation != OperationResult::SUCCESS)
	{
		while (operation != OperationResult::SUCCESS)
		{
			if (StopScan)
			{
				Writeint8_t(hPipe, Operation::STOP);
				Sleep(500);
				break;
			}
			Writeint8_t(hPipe, Operation::GET_STATE);
			operation = (OperationResult)Readint8_t(hPipe);
			Sleep(1000);
		}
		writeText("");
		Writeint8_t(hPipe, Operation::GET_STATISTICS);
		int numofParts = Readuint32_t(hPipe);

		for (int i = 0; i < numofParts; i++)
		{
			std::u16string stat = ReadU16String(hPipe);
			if (stat != u"")
				output(QDir::fromNativeSeparators(QString::fromStdU16String(stat)));
		}
		if (StopScan)
		{
			output("Scan hasn't been finished");
			StopScan = false;
		}
		else
		{
			QDateTime currTime(QDateTime::currentDateTime());
			logAppend(currTime.time().toString() + ": Finished scanning " + QString::fromStdU16String(Send));
		}
		ui->scanStatusLabel->setText("");
		setScanStartButton(true);
		setScanStopButton(false);
	}
}

void MainWindow::on_browseButton_clicked()
{
	FileDialog* fd = new FileDialog(nullptr);
	fd->show();
	if (fd->exec())
	{
		QString directory = fd->selectedFiles()[0];
		if (directory != "")
		{
			directory.replace(QString("//"), QString("/"));
			ui->pathLineEdit->setText(directory);
		}
	}
}


void MainWindow::on_startScanButton_clicked()
{
	if (ui->pathLineEdit->text() == "")
		return;
	ui->resultTextEdit->setText("");
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() + ": Started scanning " + ui->pathLineEdit->text());

	QThread* scThread = QThread::create(&MainWindow::scan, this,0);	
	ui->stopButton->setEnabled(true);
	ui->startScanButton->setEnabled(false);
	scThread->start();
}

void MainWindow::on_setButton_clicked()
{
	if (ui->pathLineEdit_2->text() == "")
		return;

	ui->setButton->setEnabled(false);
	ui->cancelSchedule->setEnabled(true);
	ui->resultTextEdit_2->setText("");
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() + ": Started scheduling " + ui->pathLineEdit_2->text());
	QThread* scThread = QThread::create(&MainWindow::startScheduling, this);
	scThread->start();
}

void MainWindow::on_browseButton_2_clicked()
{
	FileDialog* fd = new FileDialog(nullptr);
	fd->show();
	if (fd->exec())
	{
		QString directory = fd->selectedFiles()[0];
		if (directory != "")
		{
			directory.replace(QString("//"), QString("/"));
			ui->pathLineEdit_2->setText(directory);
		}
	}
}

void MainWindow::on_browseButton_3_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Choose folder to monitor", QString());
	ui->pathLineEdit_3->setText(dir);

}

void MainWindow::on_monitorButton_clicked()
{
	if (ui->pathLineEdit_3->text() == "")
		return;

	ui->monitorButton->setEnabled(false);
	ui->resultTextEdit_3->setText("");
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() +": Started monitoring " + ui->pathLineEdit_3->text());
	QThread* scThread = QThread::create(&MainWindow::startMonitoring, this);
	scThread->start();
}

void MainWindow::on_stopButton_clicked()
{
	StopScan = true;
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() + ": Stopped scanning");
	ui->stopButton->setEnabled(false);
	ui->startScanButton->setEnabled(true);
}

void MainWindow::on_cancelSchedule_clicked()
{
	CancelScheduleTime = true;
	ui->setButton->setEnabled(true);
	ui->cancelSchedule->setEnabled(false);
}

void MainWindow::on_stopButton_2_clicked()
{
	StopScheduleScan = true;
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() + ": Stopped scheduled scanning");
	ui->stopButton_2->setEnabled(false);
	ui->setButton->setEnabled(true);
}

void MainWindow::on_cancelMonitorButton_clicked()
{
	StopMonitoring = true;
	ui->cancelMonitorButton->setEnabled(false);
	QDateTime currTime(QDateTime::currentDateTime());
	ui->logTextEdit->append(currTime.time().toString() + ": Canceled monitoring ");
}

void MainWindow::updateTime()
{
	QTime qtime(QTime::currentTime().hour(), QTime::currentTime().minute(), 0, 0);
	ui->dateTimeEdit->setMinimumTime(qtime);
}
