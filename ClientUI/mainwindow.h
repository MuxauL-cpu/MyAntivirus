#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTextCodec>
#include "FileDialog.h"
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseButton_clicked();

    void on_startScanButton_clicked();

    void on_setButton_clicked();

    void on_browseButton_2_clicked();

    void on_browseButton_3_clicked();

    void on_monitorButton_clicked();

    void on_stopButton_clicked();

    void on_cancelSchedule_clicked();

    void on_stopButton_2_clicked();

    void on_cancelMonitorButton_clicked();

    void updateTime();

private:
    Ui::MainWindow *ui;
    QTimer* tmr;
    HANDLE hPipe;
	HANDLE hPipeScheduled;
    HANDLE hPipeMonitor;

	LPCTSTR lpszPipeName = TEXT("\\\\.\\pipe\\IPCPipe");
	LPCTSTR lpszScPipeName = TEXT("\\\\.\\pipe\\IPCScheduledPipe");
	LPCTSTR lpszPipeMonitorName = TEXT("\\\\.\\pipe\\IPCMonitorPipe");

    HANDLE hEvent;


    bool StopScan = false;
	bool StopScheduleScan = false;
    bool CancelScheduleTime = false;
    bool StopMonitoring = false;

private:
    void connectPipe();
    void scan(bool scheduled);
    void startScheduling();
    void startMonitoring();

signals:
	void output(const QString& toAppend);
    void setScanStopButton(bool b);
    void setScanStartButton(bool b);
	void setScheduleStopButton(bool b);
	void setScheduleSetButton(bool b);
	void setScheduleCancelButton(bool b);

	void setStartMonitoringButton(bool b);
	void setCancelMonitoringButton(bool b);

    void writeText(const QString& toWrite);
	void outputScheduled(const QString& toAppend);
	void writeTextScheduled(const QString& toWrite);
	void outputMonitor(const QString& toAppend);
	void writeTextMonitor(const QString& toWrite);
	void logAppend(const QString& toAppend);

};
#endif // MAINWINDOW_H
