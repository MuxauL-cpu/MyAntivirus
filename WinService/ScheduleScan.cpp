#include "ScheduleScan.h"
#include <chrono>
#include <ctime>
using namespace std::chrono;
std::condition_variable cv;

std::mutex cv_mutex1, cv_mutex2;

void ScheduleScannerTask::schedule(int64_t scTime,std::filesystem::path path)
{
	duration<int64_t> dur(scTime);
	scheduledTime = dur;
	currTime = duration_cast<seconds>(system_clock::now().time_since_epoch());
	int minute = std::chrono::duration_cast<std::chrono::minutes>(currTime).count();
	int minuteScheduled = std::chrono::duration_cast<std::chrono::minutes>(scheduledTime).count();
	if (minuteScheduled < minute)
	{
		_ts = TaskStatus::Failed;
		return;
	}
	_ts = TaskStatus::Scheduled;
	std::chrono::seconds delta = scheduledTime - currTime;
	std::unique_lock<std::mutex> lk(cv_mutex1);

	cv.wait_for(lk, delta, [&]() 
		{
			return Cancel; 
		});

	if (Cancel)
	{
		Cancel = false;
		_ts = TaskStatus::Complete;
		return;
	}
	scan(path);
}


void ScheduleScannerTask::cancel()
{
	Cancel = true;
	cv.notify_one();
}
