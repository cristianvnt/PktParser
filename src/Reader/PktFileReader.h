#ifndef PKT_FILE_READER_H
#define PKT_FILE_READER_H

#include <string>
#include <vector>
#include <optional>
#include <cstring>

#include "Misc/Define.h"
#include "BitReader.h"
#include "Enums/Direction.h"

namespace PktParser::Reader
{
	struct PktFileHeader
	{
		uint16 version;
		uint8 snifferId;
		uint32 clientBuild;
		char locale[4];
		uint32 startTime;
		uint32 startTickCount;
		int16 snifferVersion;
	};

	struct PktHeader
	{
		Enums::Direction direction;
		int32 connectionIndex;
		uint32 tickCount;
		int32 packetLength;
		double timestamp;
		uint32 opcode;
	};

	struct Pkt
	{
		PktHeader header;
		std::vector<uint8> data;
		uint32 pktNumber{};
		BitReader CreateReader() const { return BitReader(data.data(), data.size()); }
	};

	class PktFileReader
	{
	private:
		int _fd;
		const uint8* _mappedData;
		size_t _fileSize;
		size_t _position;

		std::string _filepath;
		PktFileHeader _fileHeader;
		uint32 _pktNumber;

	public:
		explicit PktFileReader(std::string const& filepath);
		~PktFileReader();

		PktFileReader(PktFileReader const&) = delete;
		PktFileReader& operator=(PktFileReader const&) = delete;

		void ParseFileHeader();
		std::optional<Pkt> ReadNextPacket();

		PktFileHeader const& GetFileHeader() const { return _fileHeader; }
		uint32 GetBuildVersion() const { return _fileHeader.clientBuild; }
		int GetPacketNumber() const { return _pktNumber; }
		bool IsOpen() const { return _mappedData != nullptr; }
		std::string const& GetFilePath() const { return _filepath; }
		size_t GetFileSize() const { return _fileSize; }
		uint32 GetStartTime() const { return _fileHeader.startTime; }

	private:
		PktHeader ParsePacketHeader();
		void ParsePacketAdditionalData(int32 packetAdditionalSize, double& outTimestamp);

		template<typename T>
		T Read()
		{
			if (_position + sizeof(T) > _fileSize)
				throw ParseException{ "Read past EOF" };

			T value;
			std::memcpy(&value, _mappedData + _position, sizeof(T));
			_position += sizeof(T);
			return value;
		}

		void ReadInto(void* dst, size_t count)
		{
			if (_position + count > _fileSize)
				throw ParseException{ "Read past EOF" };

			std::memcpy(dst, _mappedData + _position, count);
			_position += count;
		}

		void Skip(size_t bytes)
		{
			if (_position + bytes > _fileSize)
				throw ParseException{ "Read past EOF" };
				
			_position += bytes;
		}

		bool AtEnd() const { return _position >= _fileSize; }
	};
}

#endif // !PKT_FILE_READER_H
