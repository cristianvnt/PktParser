#include "BitReader.h"
#include "Misc/Exceptions.h"

#include <cstring>
#include <algorithm>

namespace PktParser::Reader
{
	BitReader::BitReader(uint8 const* data, size_t length)
		: _data{ data }, _length{ length }, _bytePos{ 0 }, _bitPos{ 8 }, _curBitVal{ 0 }
	{
	}

	uint8 BitReader::ReadUInt8()
	{
		ResetBitReader();

		if (_bytePos >= _length)
			throw EndOfStreamException{};

		uint8 value = _data[_bytePos];
		_bytePos++;
		return value;
	}

	uint16 BitReader::ReadUInt16()
	{
		ResetBitReader();

		if (_bytePos + sizeof(uint16) > _length)
			throw EndOfStreamException{};

		uint16 value;
		std::memcpy(&value, &_data[_bytePos], sizeof(uint16));
		_bytePos += sizeof(uint16);
		return value;
	}

	uint32 BitReader::ReadUInt32()
	{
		ResetBitReader();

		if (_bytePos + sizeof(uint32) > _length)
			throw EndOfStreamException{};

		uint32 value;
		std::memcpy(&value, &_data[_bytePos], sizeof(uint32));
		_bytePos += sizeof(uint32);
		return value;
	}

	uint64 BitReader::ReadUInt64()
	{
		ResetBitReader();

		if (_bytePos + sizeof(uint64) > _length)
			throw EndOfStreamException{};

		uint64 value;
		std::memcpy(&value, &_data[_bytePos], sizeof(uint64));
		_bytePos += sizeof(uint64);
		return value;
	}

	int32 BitReader::ReadInt32()
	{
		return static_cast<int32>(ReadUInt32());
	}

	float BitReader::ReadFloat()
	{
		ResetBitReader();

		if (_bytePos + sizeof(float) > _length)
			throw EndOfStreamException{};

		float value;
		std::memcpy(&value, &_data[_bytePos], sizeof(float));
		_bytePos += sizeof(float);
		return value;
	}

	std::string BitReader::ReadWoWString(uint32 len /*= 0*/)
	{
		if (len == 0)
			return "";

		std::vector<uint8> bytes(len);
		for (uint32 i = 0; i < len; ++i)
			bytes[i] = ReadUInt8();

		auto newEnd = std::remove_if(bytes.begin(), bytes.end(), [](uint8 b) { return b != 0; });
		return std::string(bytes.begin(), newEnd);
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

	bool BitReader::ReadBit()
	{
		if (_bitPos == 8)
		{
			if (_bytePos >= _length)
				throw EndOfStreamException{};

			_bitPos = 0;
			_curBitVal = _data[_bytePos];
			_bytePos++;
		}

		bool bit = ((_curBitVal >> (7 - _bitPos)) & 1) != 0;
		++_bitPos;
		return bit;
	}

	void BitReader::ResetBitReader()
	{
		_bitPos = 8;
	}

	uint8 const* BitReader::GetCurrentPtr() const
	{
		return &_data[_bytePos];
	}

	void BitReader::Skip(size_t bytes)
	{
		if (_bytePos + bytes > _length)
			throw EndOfStreamException{};

		_bytePos += bytes;
	}

	bool BitReader::CanRead() const
	{
		return _bytePos < _length;
	}

}