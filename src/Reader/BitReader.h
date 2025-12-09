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

	template<typename T>
	inline T const* ReadChunk()
	{
		static_assert(std::is_trivially_copyable_v<T>, "T mult be trivially copyable");
		static_assert(std::is_standard_layout_v<T>, "T must have standard layout");
		ResetBitReader();

		T const* result = reinterpret_cast<T const*>(GetCurrentPtr());
		Skip(sizeof(T));
		return result;
	}

	template<typename T>
	inline void ReadChunkArray(std::vector<T>& out, size_t count)
	{
		static_assert(std::is_trivially_copyable_v<T>, "T mult be trivially copyable");
		ResetBitReader();

		if (count == 0)
		{
			out.clear();
			return;
		}

		out.resize(count);
		std::memcpy(out.data(), GetCurrentPtr(), count * sizeof(T));
		Skip(count * sizeof(T));
	}
};

#endif // !BIT_READER_H
