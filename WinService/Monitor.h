#pragma once
#include "ScanBase.h"
#include <windows.h>

class Monitor : public ScanTaskBase
{
public:
	Monitor(Scanner& scanner) : ScanTaskBase(scanner) 
	{
		hEvent = CreateEvent(NULL, FALSE, FALSE, L"StopMonitoringEvent");

	}
	void monitorFolder(std::filesystem::path path);
	void stop() override;

private:
	HANDLE dirHandle;
	HANDLE hEvent;
	bool toStop;
};

