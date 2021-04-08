#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "Scanner.h"

Scanner::Scanner(Bases &base)
{
	scEngine = std::make_unique<ScanEngine>(base);
}
using namespace zipper;

void Scanner::Stop() 
{
	scEngine->ScanStop();
}

void Scanner::Scan(const std::filesystem::path& path)
{
	locked = true;
	mtx.lock();
	Sleep(1000);
	statistics = u"";
	ScanObject scanObj;
	scanObj.setPath(path);
	scEngine->ScanPath(scanObj,statistics);
	mtx.unlock();
	locked = false;
}
