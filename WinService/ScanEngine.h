#pragma once
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <zipper/zipper.h>
#include <zipper/unzipper.h>
#include "Base.h"
#include "ScanObject.h"

#if _DEBUG
#pragma comment(lib,"Zipper-staticd.lib")
#else
#pragma comment(lib,"Zipper-static.lib")
#endif
#define MINSIGLENGTH 8

class ScanEngine
{
public:
	ScanEngine(const Bases& base)
		:
		base(base)
	{
		contents.resize(bufferSize);
	}
	void ScanZip(ScanObject scanObj, std::u16string& stat);
	void ScanPath(ScanObject scanObj, std::u16string& stat);
	void ScanFolder(ScanObject scanObj, std::u16string& stat);
	void ScanFile(ScanObject scanObj, std::u16string &stat);
	void ScanStop();
private:
	std::filesystem::path path;
	std::string contents;
	uint64_t bufferSize = 1 << 20;
	std::u16string virusName;
	Bases base;
	bool stopped;
private:
	void updateString(std::ifstream& ifs);
	void updateString(std::istream& is);
};

