#ifndef BIT_READER_H
#define BIT_READER_H

#include "Misc/Define.h"
#include "Misc/Exceptions.h"

#include <vector>
#include <string>
#include <cstring>

namespace PktParser::Reader
{
	class BitReader
	{
	private:
		uint8 const* _data;
		size_t _length;
		size_t _bytePos;
		uint8 _bitPos;
		uint8 _curBitVal;

	public:
		BitReader(uint8 const* data, size_t length)
			: _data{ data }, _length{ length }, _bytePos{ 0 }, _bitPos{ 8 }, _curBitVal{ 0 } {}

		void ResetBitReader()
		{
			_bitPos = 8;
		}

		uint8 ReadUInt8()
		{
			ResetBitReader();

			if (_bytePos >= _length)
				throw ParseException{"Read UInt8"};

			uint8 value = _data[_bytePos];
			_bytePos++;
			return value;
		}

		uint16 ReadUInt16()
		{
			ResetBitReader();

			if (_bytePos + sizeof(uint16) > _length)
				throw ParseException{"Read UInt16"};

			uint16 value;
			std::memcpy(&value, &_data[_bytePos], sizeof(uint16));
			_bytePos += sizeof(uint16);
			return value;
		}

		uint32 ReadUInt32()
		{
			ResetBitReader();

			if (_bytePos + sizeof(uint32) > _length)
				throw ParseException{"Read UInt32"};

			uint32 value;
			std::memcpy(&value, &_data[_bytePos], sizeof(uint32));
			_bytePos += sizeof(uint32);
			return value;
		}

		uint64 ReadUInt64()
		{
			ResetBitReader();

			if (_bytePos + sizeof(uint64) > _length)
				throw ParseException{"Read UInt64"};

			uint64 value;
			std::memcpy(&value, &_data[_bytePos], sizeof(uint64));
			_bytePos += sizeof(uint64);
			return value;
		}

		int32 ReadInt32()
		{
			return static_cast<int32>(ReadUInt32());
		}

		float ReadFloat()
		{
			ResetBitReader();

			if (_bytePos + sizeof(float) > _length)
				throw ParseException{"Read Float"};

			float value;
			std::memcpy(&value, &_data[_bytePos], sizeof(float));
			_bytePos += sizeof(float);
			return value;
		}

		bool ReadBit()
		{
			if (_bitPos == 8)
			{
				if (_bytePos >= _length)
					throw ParseException{"Read Bit"};

				_bitPos = 0;
				_curBitVal = _data[_bytePos];
				_bytePos++;
			}

			bool bit = ((_curBitVal >> (7 - _bitPos)) & 1) != 0;
			++_bitPos;
			return bit;
		}

		std::string ReadWoWString(uint32 len = 0);
		uint32 ReadBits(uint8 numBits);

		size_t GetBytePosition() const { return _bytePos; }
		size_t GetLength() const { return _length; }
		uint8 const* GetCurrentPtr() const { return &_data[_bytePos]; }
		bool CanRead() const { return _bytePos < _length; }
		
		void Skip(size_t bytes)
		{
			if (_bytePos + bytes > _length)
				throw ParseException{"Skip"};

			_bytePos += bytes;
		}

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
}

#endif // !BIT_READER_H
