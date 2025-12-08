#ifndef PKT_FILE_READER_H
#define PKT_FILE_READER_H

#include <fstream>
#include <string>
#include <vector>
#include <optional>

#include "Misc/Define.h"
#include "BitReader.h"

namespace PktParser
{
	// file header
	struct PktFileHeader
	{
		uint16 version;
		uint8 snifferId;
		uint32 clientBuild;
		std::string locale;
		uint32 startTime;
		uint32 startTickCount;
		int16 snifferVersion;
	};

	// packet header
	struct PktHeader
	{
		std::string direction;
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
		BitReader CreateReader() const { return BitReader(data.data(), data.size()); }
	};

	class PktFileReader
	{
	private:
		std::ifstream _file;
		PktFileHeader _fileHeader;
		int _pktNumber;

	public:
		explicit PktFileReader(std::string const& filepath);
		~PktFileReader();

		void ParseFileHeader();

		std::optional<Pkt> ReadNextPacket();

		PktFileHeader const& GetFileHeader() const { return _fileHeader; }
		int GetPacketNumber() const { return _pktNumber; }
		bool IsOpen() const { return _file.is_open(); }

	private:
		PktHeader ParsePacketHeader();
		void ParsePacketAdditionalData(int32 packetAdditionalSize, double& outTimestamp);
	};
}

#endif // PKT_FILE_READER_H
