#include "BitReader.h"
#include "Misc/Exceptions.h"

#include <cstring>

BitReader::BitReader(uint8 const* data, size_t length)
	: _data{ data }, _length{ length }, _bytePos{ 0 }, _bitPos{ 0 }
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

uint32 BitReader::ReadBits(uint8 numBits)
{
	if (!numBits || numBits > 32)
		throw ParseException{ "Invalid bit count: must be 1-32" };

	uint32 result = 0;
	uint8 bitsRead = 0;

	while (bitsRead < numBits)
	{
		if (_bytePos >= _length)
			return result;

		uint8 bitsLeftInByte = 8 - _bitPos;
		uint8 bitsNeeded = numBits - bitsRead;

		uint8 bitsToRead = (bitsNeeded < bitsLeftInByte) ? bitsNeeded : bitsLeftInByte;

		uint8 mask = (1 << bitsToRead) - 1;

		uint8 value = (_data[_bytePos] >> _bitPos) & mask;

		result |= static_cast<uint32>(value) << bitsRead;

		_bitPos += bitsToRead;
		bitsRead += bitsToRead;

		if (_bitPos >= 8)
		{
			_bytePos++;
			_bitPos = 0;
		}
	}

	return result;
}

bool BitReader::ReadBit()
{
	return ReadBits(1) != 0;
}

void BitReader::ResetBitReader()
{
	if (_bitPos != 0)
	{
		_bytePos++;
		_bitPos = 0;
	}
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
