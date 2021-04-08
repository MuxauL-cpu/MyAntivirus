#pragma once
#include <cstdint>
#include <windows.h>
#include <string>
namespace codes
{
	enum codes { INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64, PATH };
}
int8_t Readint8_t(HANDLE hPipe);
void Writeint8_t(HANDLE hPipe, int8_t);

int16_t Readint16_t(HANDLE hPipe);
void Writeint16_t(HANDLE hPipe, int16_t num);

int32_t Readint32_t(HANDLE hPipe);
void Writeint32_t(HANDLE hPipe, int32_t);

int64_t Readint64_t(HANDLE hPipe);
void Writeint64_t(HANDLE hPipe, int64_t num);

uint8_t Readuint8_t(HANDLE hPipe);
void Writeuint8_t(HANDLE hPipe, uint8_t num);

uint16_t Readuint16_t(HANDLE hPipe);
void Writeuint16_t(HANDLE hPipe, uint16_t num);

uint32_t Readuint32_t(HANDLE hPipe);
void Writeuint32_t(HANDLE hPipe, uint32_t num);

uint64_t Readuint64_t(HANDLE hPipe);
void Writeuint64_t(HANDLE hPipe, uint64_t num);

void ReadPath(HANDLE hPipe, TCHAR* buf);
void WritePath(HANDLE hPipe, TCHAR* buf);

std::u16string ReadU16String(HANDLE hPipe);
void WriteU16String(HANDLE hPipe, std::u16string str);


