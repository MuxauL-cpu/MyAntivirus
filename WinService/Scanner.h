#pragma once
#include "Base.h"
#include <filesystem>
#include <algorithm>
#include <string>
#include "ScanEngine.h"
#include "ScanObject.h"
#include <zipper/zipper.h>
#include <zipper/unzipper.h>
#include <mutex>

class Scanner
{
public:
	Scanner(Bases& base);
	void Scan(const std::filesystem::path& path);
	void Stop();
	std::u16string getStatistics() { return statistics; }
	bool isLocked() { return locked; }
private:
	Bases base;
	std::unique_ptr<ScanEngine> scEngine;
	static inline std::mutex mtx;
	std::u16string statistics;
	bool locked = false;
};