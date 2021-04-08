#pragma once
#include "Scanner.h"
#include <thread>
#include <chrono>

enum TaskStatus
{
	Stopped,
	Running,
	Complete,
	Scheduled,
	Monitoring,
	Failed,
	Waiting
};

class ScanTaskBase
{
protected:
	Scanner* _sc;
	TaskStatus _ts;
	std::u16string _taskStatistic;
public:
	ScanTaskBase(Scanner& scanner)
	{
		_taskStatistic = u"";
		_sc = &scanner;
	}

	virtual void scan(std::filesystem::path path) 
	{
		_ts = TaskStatus::Waiting;
		_taskStatistic = u"";
		while (_sc->isLocked())
		{
			if (_ts == TaskStatus::Stopped)
				return;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		_ts = TaskStatus::Running;
		_sc->Scan(path);
		_taskStatistic = _sc->getStatistics();
		_ts = TaskStatus::Complete;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	virtual void stop()
	{

		if (_ts == TaskStatus::Running)
		{
			_sc->Stop();
			_ts = TaskStatus::Stopped;
		}
		else if (_ts == TaskStatus::Waiting)
		{
			_ts = TaskStatus::Stopped;
		}
	}

	TaskStatus getStatus()
	{
		return _ts;
	}

	std::u16string getTaskStatistic()
	{
		return _taskStatistic;
	}
};