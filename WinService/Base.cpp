#include "Base.h"
#include "BaseLoad.h"
Bases::Bases(const std::u16string path)
{
	Base = BaseLoader::Load(path);
}

bool Bases::find(uint8_t* begin, uint32_t offStart,std::u16string& name)
{
	uint64_t key = *((uint64_t*)(begin));
	std::unordered_map<uint64_t, Record>::iterator got = Base.find(key);

	if (got != Base.end())
	{
		if(offStart == got->second.offsetStart && (offStart + got->second.length + MINSIGLENGTH - 1 <= got->second.offsetEnd))
		{
			SHA256 sha;
			std::vector<uint8_t> tempforHash;
			tempforHash.resize(got->second.length);
			memcpy(&tempforHash[0], begin + MINSIGLENGTH, got->second.length);
			sha.update(&tempforHash[0],tempforHash.size());
			uint8_t* digest = sha.digest();
			std::u16string hash = SHA256::toU16String(digest);
			delete[] digest;
			if (got->second.sha256 == hash)
			{
				name = got->second.name;
				return true;
			}
		}
	}
	return false;
}

void Bases::updateBase(const std::u16string path)
{
	Base = BaseLoader::Load(path);
}
