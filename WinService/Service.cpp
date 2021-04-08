#define _CRT_SECURE_NO_WARNINGS
#include "Service.h"
#include <iostream>
#include <strsafe.h>
#include <filesystem>
#include <WtsApi32.h>
#include <userenv.h>
#include "Scanner.h"
#include <future>
#include <aclapi.h>
#include <IPC.h>
#include <Operations.h>
#include "ScanPath.h"
#include <tchar.h>
#include "ScheduleScan.h"
#include <thread>
#pragma comment(lib,"wtsapi32.lib")
#pragma comment(lib,"userenv.lib")
#define BUFSIZE 2048
Service::Service()
{

}

void Service::ServiceProcess(int argc, TCHAR* argv[])
{
	if (lstrcmp(argv[1], TEXT("install")) == 0)
	{
		ServiceInstance->SvcInstall();
	}
	if (lstrcmp(argv[1], TEXT("update")) == 0)
	{
		ServiceInstance->DoUpdateSvcDacl();
	}
	if (lstrcmp(argv[1], TEXT("uninstall")) == 0)
	{
		ServiceInstance->SvcUninstall();
	}
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{SVCNAME,(LPSERVICE_MAIN_FUNCTION)Service::MainProcess},
		{NULL,NULL}
	};
	ServiceInstance->startSvc();
	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
	}
}

void Service::SvcInstall()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];
	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	schService = CreateService(schSCManager, SVCNAME, SVCNAME,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL, szPath, NULL, NULL, NULL, NULL, NULL);
	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service installed successfully\n");
	createRegistryRecord();
	DoUpdateSvcDacl();
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

void Service::SvcUninstall()
{
	DWORD dwBytesNeeded;
	SERVICE_STATUS_PROCESS ssp;

	SC_HANDLE schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	SC_HANDLE schService = OpenService(
		schSCManager,         // SCM database 
		SVCNAME,            // name of service 
		SERVICE_ALL_ACCESS |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	DeleteService(schService);
	deleteRegistryRecord();
}

void Service::setServiceInstance(Service* Instance)
{
	if (Instance)
		ServiceInstance = Instance;
}

void Service::MainProcess()
{
	ServiceInstance->gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, (LPHANDLER_FUNCTION)Service::SvcCtrlHandler);
	if (!ServiceInstance->gSvcStatusHandle)
	{
		SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
		return;
	}
	ServiceInstance->gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceInstance->gSvcStatus.dwServiceSpecificExitCode = 0;
	SetServiceStatus(ServiceInstance->gSvcStatusHandle, &(ServiceInstance->gSvcStatus));
	ServiceInstance->ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	ServiceInstance->SvcInit();
}

VOID Service::SvcReportEvent(LPTSTR szFunction)
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	TCHAR buffer[80];
	hEventSource = RegisterEventSource(NULL, SVCNAME);
	if (NULL != hEventSource)
	{
		StringCchPrintf(buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());
		lpszStrings[0] = SVCNAME;
		lpszStrings[1] = buffer;
		ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, SVC_ERROR, NULL, 2, 0, lpszStrings, NULL);
		DeregisterEventSource(hEventSource);
	}
}

VOID WINAPI Service::SvcCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{

	case SERVICE_CONTROL_INTERROGATE:
		break;
	case CTRL_CLOSE_EVENT:
		break;
	default:
		break;
	}
}

VOID Service::ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}


void Service::createRegistryRecord()
{
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	DWORD dwDisposition;
	HKEY  hKey;
	DWORD Ret;
	Ret =
		RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			TEXT("SOFTWARE\\ClientUI"),
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS | KEY_WOW64_64KEY,
			NULL,
			&hKey,
			&dwDisposition);

	if (Ret != ERROR_SUCCESS)
	{
		printf("Error opening or creating new key\n");
		return;
	}

	RegSetValueEx(hKey,
		TEXT("Working Directory"),
		0,
		REG_SZ,
		(LPBYTE)(buffer),
		((((DWORD)lstrlen(buffer) + 1)) * sizeof(TCHAR)));

	RegCloseKey(hKey);
}

void Service::setWorkingDirectory()
{
	DWORD len = 1024;
	DWORD readDataLen = len;

	DWORD Ret;
	HKEY hKey;

	Ret = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\ClientUI"),
		0,
		KEY_READ | KEY_WOW64_64KEY,
		&hKey
	);

	Ret = RegQueryValueEx(
		hKey,
		TEXT("Working Directory"),
		NULL,
		NULL,
		(BYTE*)WorkingDirectory,
		&readDataLen
	);
	if (Ret != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return;
	}
	RegCloseKey(hKey);

	_tcscat(WorkingDirectory, TEXT("\\"));
	SetCurrentDirectory(WorkingDirectory);

	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
}

void Service::deleteRegistryRecord()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ClientUI"),
		0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		RegDeleteTree(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ClientUI"));
	}
}


VOID Service::startSvc()
{
	SERVICE_STATUS_PROCESS ssStatus;
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	schService = OpenService(schSCManager, SVCNAME, SERVICE_ALL_ACCESS);


	StartServiceA(schService, NULL, NULL);

	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);

}

VOID Service::SvcInit()
{
	setWorkingDirectory();
	ServiceInstance->ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
	serv.launchUI();

	std::thread t1(&Server::WaitForPipe, &serv);
 	t1.join();
 	std::thread t2(&Server::AcceptMessages, &serv);
 	t2.detach();
}



VOID WINAPI Service::DoUpdateSvcDacl(void)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	EXPLICIT_ACCESS      ea;
	SECURITY_DESCRIPTOR  sd;
	PSECURITY_DESCRIPTOR psd = NULL;
	PACL                 pacl = NULL;
	PACL                 pNewAcl = NULL;
	BOOL                 bDaclPresent = FALSE;
	BOOL                 bDaclDefaulted = FALSE;
	DWORD                dwError = 0;
	DWORD                dwSize = 0;
	DWORD                dwBytesNeeded = 0;
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service

	schService = OpenService(
		schSCManager,              // SCManager database 
		SVCNAME,                 // name of service 
		READ_CONTROL | WRITE_DAC); // access

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Get the current security descriptor.

	if (!QueryServiceObjectSecurity(schService,
		DACL_SECURITY_INFORMATION,
		&psd,           // using NULL does not work on all versions
		0,
		&dwBytesNeeded))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			dwSize = dwBytesNeeded;
			psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, dwSize);
			if (psd == NULL)
			{
				// Note: HeapAlloc does not support GetLastError.
				printf("HeapAlloc failed\n");
				goto dacl_cleanup;
			}

			if (!QueryServiceObjectSecurity(schService,
				DACL_SECURITY_INFORMATION, psd, dwSize, &dwBytesNeeded))
			{
				printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
				goto dacl_cleanup;
			}
		}
		else
		{
			printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
			goto dacl_cleanup;
		}
	}

	// Get the DACL.

	if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl,
		&bDaclDefaulted))
	{
		printf("GetSecurityDescriptorDacl failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Build the ACE.
	memset(&ea, 0, sizeof(ea));
	BuildExplicitAccessWithName(&ea, TEXT("CURRENT_USER"),
		SERVICE_START,
		SET_ACCESS, NO_INHERITANCE);

	dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
	if (dwError != ERROR_SUCCESS)
	{
		printf("SetEntriesInAcl failed(%d)\n", dwError);
		goto dacl_cleanup;
	}

	// Initialize a new security descriptor.

	if (!InitializeSecurityDescriptor(&sd,
		SECURITY_DESCRIPTOR_REVISION))
	{
		printf("InitializeSecurityDescriptor failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Set the new DACL in the security descriptor.

	if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))
	{
		printf("SetSecurityDescriptorDacl failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Set the new DACL for the service object.

	if (!SetServiceObjectSecurity(schService,
		DACL_SECURITY_INFORMATION, &sd))
	{
		printf("SetServiceObjectSecurity failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}
	else printf("Service DACL updated successfully\n");

dacl_cleanup:
	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);

	if (NULL != pNewAcl)
		LocalFree((HLOCAL)pNewAcl);
	if (NULL != psd)
		HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
}
