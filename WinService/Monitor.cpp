#include "Monitor.h"

void Monitor::monitorFolder(std::filesystem::path path)
{
	dirHandle = FindFirstChangeNotification(path.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);

	HANDLE handles[2] = { dirHandle, hEvent };
	while (true)
	{
		_ts = TaskStatus::Monitoring;
		DWORD Check = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		if (toStop)
		{
			toStop = false;
			_ts = TaskStatus::Stopped;
			CloseHandle(handles[0]);
			return;
		}
		Sleep(1000);
		scan(path);
		CloseHandle(handles[0]);
		handles[0] = FindFirstChangeNotification(path.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
	}
}

void Monitor::stop()
{
	toStop = true;
	SetEvent(hEvent);
}
	