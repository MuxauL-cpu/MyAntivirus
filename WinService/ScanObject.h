#pragma once
#include <sstream>
#include <filesystem>

enum ObjType 
{
	OBJFILE,OBJDIR,OBJZIP
};

class ScanObject
{
public:
	ScanObject() = default;
	ScanObject(std::filesystem::path& path) : path(path) {};
	std::stringstream* sstream;

	void setPath(const std::filesystem::path &path);
	std::filesystem::path getPath() 
	{
		return path; 
	}

	ObjType type;

private:
	std::filesystem::path path;
};

