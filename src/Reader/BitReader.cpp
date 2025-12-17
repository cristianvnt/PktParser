#include "pchdef.h"

namespace PktParser::Reader
{
	std::string BitReader::ReadWoWString(uint32 len /*= 0*/)
	{
		if (len == 0)
			return {};

		uint8 const* ptr = GetCurrentPtr();
		Skip(len);
		uint8 const* newEnd = std::find(ptr, ptr + len, 0);

		return std::string(reinterpret_cast<char const*>(ptr), newEnd - ptr);
	}

	uint32 BitReader::ReadBits(uint8 numBits)
	{
		if (!numBits || numBits > 32)
			throw ParseException{ "Invalid bit count: must be 1-32" };

		uint32 value = 0;
		for (int i = numBits - 1; i >= 0; --i)
			if (ReadBit())
				value |= (uint32)(1 << i);

		return value;
	}
}