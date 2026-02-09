#ifndef PKT_FILE_READER_H
#define PKT_FILE_READER_H

#include <fstream>
#include <string>
#include <vector>
#include <optional>

#include "Misc/Define.h"
#include "BitReader.h"
#include "Enums/Direction.h"

namespace PktParser::Reader
{
	// file header
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

	// packet header
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
		std::ifstream _file;
		std::string _filepath;
		PktFileHeader _fileHeader;
		uint32 _pktNumber;
		size_t _fileSize;

	public:
		explicit PktFileReader(std::string const& filepath);
		~PktFileReader();

		void ParseFileHeader();
		std::optional<Pkt> ReadNextPacket();

		PktFileHeader const& GetFileHeader() const { return _fileHeader; }
		uint32 GetBuildVersion() const { return _fileHeader.clientBuild; }
		int GetPacketNumber() const { return _pktNumber; }
		bool IsOpen() const { return _file.is_open(); }
		std::string const& GetFilePath() const { return _filepath; }
		size_t GetFileSize() const { return _fileSize; }
		uint32 GetStartTime() const { return _fileHeader.startTime; }

	private:
		PktHeader ParsePacketHeader();
		void ParsePacketAdditionalData(int32 packetAdditionalSize, double& outTimestamp);
	};
}

#endif // !PKT_FILE_READER_H
