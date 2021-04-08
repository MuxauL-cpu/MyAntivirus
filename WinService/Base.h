#pragma once
#include <iostream>
#include <unordered_map>
#include <SHA256.h>
#include <fstream>

#define MINSIGLENGTH 8

struct Record
{
	std::u16string name, type,sha256;
	uint64_t sig, length, offsetStart, offsetEnd;

};

class Bases
{
public:
	Bases() = default;
	Bases(const std::u16string path);
	bool find(uint8_t* begin, uint32_t offStart, std::u16string& name);
	void updateBase(const std::u16string path);
private:
	std::unordered_map<uint64_t, Record> Base;

};