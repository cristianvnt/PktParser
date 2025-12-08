#ifndef BIT_READER_H
#define BIT_READER_H

#include "Misc/Define.h"

class BitReader
{
private:
	uint8 const* _data;
	size_t _length;
	size_t _bytePos;
	uint8 _bitPos;

public:
	BitReader(uint8 const* data, size_t length);

	uint8 ReadUInt8();
	uint16 ReadUInt16();
	uint32 ReadUInt32();
	uint64 ReadUInt64();
	int32 ReadInt32();
	float ReadFloat();

	uint32 ReadBits(uint8 numBits);
	bool ReadBit();

	void ResetBitReader();

	uint8 const* GetCurrentPtr() const;
	void Skip(size_t bytes);

	bool CanRead() const;
};

#endif // BIT_READER_H
