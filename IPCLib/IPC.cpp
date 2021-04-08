#include "IPC.h"
#include <iostream>
int8_t Readint8_t(HANDLE hPipe)
{
	DWORD cbRead;
	int8_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(int8_t), &cbRead, NULL);
	return buf;
}

void Writeint8_t(HANDLE hPipe, int8_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(int8_t), &cbWritten, NULL);
}

int16_t Readint16_t(HANDLE hPipe)
{
	DWORD cbRead;
	int16_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(int16_t), &cbRead, NULL);
	return buf;
}

void Writeint16_t(HANDLE hPipe, int16_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(int16_t), &cbWritten, NULL);
}

int32_t Readint32_t(HANDLE hPipe)
{
	DWORD cbRead;
	int32_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(int32_t), &cbRead, NULL);
	return buf;
}

void Writeint32_t(HANDLE hPipe, int32_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(int32_t), &cbWritten, NULL);
}

int64_t Readint64_t(HANDLE hPipe)
{
	DWORD cbRead;
	int64_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(int64_t), &cbRead, NULL);
	return buf;
}

void Writeint64_t(HANDLE hPipe, int64_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(int64_t), &cbWritten, NULL);
}

uint8_t Readuint8_t(HANDLE hPipe)
{
	DWORD cbRead;
	uint8_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(uint8_t), &cbRead, NULL);
	return buf;
}

void Writeuint8_t(HANDLE hPipe, uint8_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(uint8_t), &cbWritten, NULL);
}

uint16_t Readuint16_t(HANDLE hPipe)
{
	DWORD cbRead;
	uint16_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(uint16_t), &cbRead, NULL);
	return buf;
}
void Writeuint16_t(HANDLE hPipe, uint16_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(uint16_t), &cbWritten, NULL);
}
uint32_t Readuint32_t(HANDLE hPipe)
{
	DWORD cbRead;
	uint32_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(uint32_t), &cbRead, NULL);
	return buf;
}

void Writeuint32_t(HANDLE hPipe, uint32_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(uint32_t), &cbWritten, NULL);
}

uint64_t Readuint64_t(HANDLE hPipe)
{
	DWORD cbRead;
	uint64_t buf;
	ReadFile(hPipe, (void*)&buf, sizeof(uint64_t), &cbRead, NULL);
	return buf;
}

void Writeuint64_t(HANDLE hPipe, uint64_t num)
{
	DWORD cbWritten;
	WriteFile(hPipe, &num, sizeof(uint64_t), &cbWritten, NULL);
}

void ReadPath(HANDLE hPipe, TCHAR* buf)
{
	DWORD cbRead;
	ReadFile(hPipe, buf, MAX_PATH * sizeof(TCHAR), &cbRead, NULL);
}

void WritePath(HANDLE hPipe, TCHAR* buf)
{
	DWORD cbWritten;
	WriteFile(hPipe, buf, MAX_PATH * sizeof(TCHAR), &cbWritten, NULL);
}

std::u16string ReadU16String(HANDLE hPipe)
{
	uint16_t length = Readuint16_t(hPipe);
	DWORD cbRead;
	char16_t cstr[2048];
	ReadFile(hPipe, cstr, length*sizeof(char16_t), &cbRead, NULL);
	cstr[length] = '\0';
	return std::u16string(cstr);
}

void WriteU16String(HANDLE hPipe, std::u16string str)
{
	Writeuint16_t(hPipe, str.length());
	DWORD cbWritten;
	WriteFile(hPipe, (void*)str.c_str(), str.length()*sizeof(char16_t), &cbWritten, NULL);
}