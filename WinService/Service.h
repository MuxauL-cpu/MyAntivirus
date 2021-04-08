#pragma once
#include <windows.h>
#include "sample.h"
#include <string>
#include "ScanPath.h"
#include "ScheduleScan.h"
#include "Monitor.h"
#include "Server.h"
#ifndef NDEBUG
#define SVCNAME TEXT("ClientUIDbg")
#else
#define SVCNAME TEXT("ClientUI")
#endif // DEBUG

class Service
{
public:
	Service();
	static void ServiceProcess(int argc, TCHAR* argv[]);
	void SvcInstall();
	void SvcUninstall();
	static void setServiceInstance(Service* Instance);
private:
	static void MainProcess();
	static VOID SvcReportEvent(LPTSTR szFunction);
	static VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
	VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	VOID startSvc();
	VOID WINAPI DoUpdateSvcDacl(void);
	static inline Service* ServiceInstance;

	LPWSTR ServiceName;
	SERVICE_STATUS gSvcStatus;
	SERVICE_STATUS_HANDLE gSvcStatusHandle;

	void createRegistryRecord();
	void setWorkingDirectory();
	void deleteRegistryRecord();
	VOID SvcInit();

private:
	static inline Server serv;
	TCHAR WorkingDirectory[MAX_PATH];

};

