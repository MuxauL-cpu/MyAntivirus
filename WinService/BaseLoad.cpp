#include "BaseLoad.h"
std::unordered_map<uint64_t, Record> BaseLoader::Load(const std::u16string path)
{
	HANDLE hFile = CreateFile((wchar_t*)path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	std::u16string header = ReadU16String(hFile);

	std::unordered_map<uint64_t, Record> base;

	if (header != std::u16string(u"Maksimenko"))
		return base;
	uint16_t rowCount = Readuint16_t(hFile);
	Record record = {};

	for (int i = 0; i < rowCount; i++)
	{
		record.name = ReadU16String(hFile);
		record.type = ReadU16String(hFile);
		record.sig = Readuint64_t(hFile);
		std::reverse((uint8_t*)&record.sig,(uint8_t*)&record.sig+8);
		record.length = Readuint64_t(hFile);
		record.sha256 = ReadU16String(hFile);
		record.offsetStart = Readuint64_t(hFile);
		record.offsetEnd = Readuint64_t(hFile);
		base.insert({ record.sig,record });
	}
	CloseHandle(hFile);
	return base;
}
