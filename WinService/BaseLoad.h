#pragma once
#include <windows.h>
#include <IPC.h>
#include "Base.h"
#include <sstream>

class BaseLoader
{
public:
	static std::unordered_map<uint64_t, Record> Load(const std::u16string path);
};