#include "PktFileReader.h"
#include "Misc/Logger.h"
#include "Misc/Exceptions.h"
#include "Misc/Utilities.h"

#include <algorithm>

using namespace PktParser::Enums;

namespace PktParser::Reader
{
	PktFileReader::PktFileReader(std::string const& filepath)
		: _pktNumber{ 0 }
	{
		_file.open(filepath, std::ios::binary);

		if (!_file.is_open())
			throw ParseException{ "Failed to open: " + filepath };
	}

	PktFileReader::~PktFileReader()
	{
		if (_file.is_open())
			_file.close();
	}

	void PktFileReader::ParseFileHeader()
	{
		char magik[3];
		_file.read(magik, 3);

		if (!std::equal(magik, magik + 3, "PKT"))
			throw ParseException{ "Invalid PKT file" };

		_file.read(reinterpret_cast<char*>(&_fileHeader.version), sizeof(_fileHeader.version));

		_file.read(reinterpret_cast<char*>(&_fileHeader.snifferId), sizeof(_fileHeader.snifferId));

		_file.read(reinterpret_cast<char*>(&_fileHeader.clientBuild), sizeof(_fileHeader.clientBuild));

		_file.read(_fileHeader.locale, 4);

		_file.seekg(40, std::ios::cur); // skip session key

		_file.read(reinterpret_cast<char*>(&_fileHeader.startTime), sizeof(_fileHeader.startTime));

		_file.read(reinterpret_cast<char*>(&_fileHeader.startTickCount), sizeof(_fileHeader.startTickCount));

		int32 additionalLength;
		_file.read(reinterpret_cast<char*>(&additionalLength), sizeof(additionalLength));

		_fileHeader.snifferVersion = 0;
		if ((_fileHeader.snifferId == 0x15 || _fileHeader.snifferId == 0x16) && additionalLength >= 2)
			_file.read(reinterpret_cast<char*>(&_fileHeader.snifferVersion), sizeof(_fileHeader.snifferVersion));
		else
			_file.seekg(additionalLength, std::ios::cur);
	}

	std::optional<Pkt> PktFileReader::ReadNextPacket()
	{
		if (_file.peek() == EOF || !_file.good())
        	return std::nullopt;

		try
		{
			PktHeader header = ParsePacketHeader();

			std::vector<uint8> packetData(header.packetLength);
			_file.read(reinterpret_cast<char*>(packetData.data()), header.packetLength);

			if (_file.gcount() != header.packetLength)
				return std::nullopt;

			if (header.packetLength >= 4)
				header.opcode = static_cast<uint32>(packetData[0])
					| (static_cast<uint32>(packetData[1]) << 8)
					| (static_cast<uint32>(packetData[2]) << 16)
					| (static_cast<uint32>(packetData[3]) << 24);
			else
				header.opcode = 0;
			
			Pkt pkt{ header, std::move(packetData) };
			pkt.pktNumber = _pktNumber;
			_pktNumber++;
			return pkt;
		}
		catch (...)
		{
			return std::nullopt;
		}
	}

	PktHeader PktFileReader::ParsePacketHeader()
	{
		PktHeader header{};

		uint32 directionMagik;
		_file.read(reinterpret_cast<char*>(&directionMagik), sizeof(directionMagik));

		switch (directionMagik)
		{
		case 0x47534D53: // "SMSG"
			header.direction = Direction::ServerToClient;
			break;
		case 0x47534D43: // "CMSG"
			header.direction = Direction::ClientToServer;
			break;
		case 0x4E425F53: // "S_BN"
			header.direction = Direction::BNServerToClient;
			break;
		case 0x43425F53: // "S_BC"
			header.direction = Direction::BNClientToServer;
			break;
		default:
			header.direction = Direction::Bidirectional;
			break;
		}

		_file.read(reinterpret_cast<char*>(&header.connectionIndex), sizeof(header.connectionIndex));
		_file.read(reinterpret_cast<char*>(&header.tickCount), sizeof(header.tickCount));

		int32 packetAdditionalSize;
		_file.read(reinterpret_cast<char*>(&packetAdditionalSize), sizeof(packetAdditionalSize));

		_file.read(reinterpret_cast<char*>(&header.packetLength), sizeof(header.packetLength));

		ParsePacketAdditionalData(packetAdditionalSize, header.timestamp);

		return header;
	}

	void PktFileReader::ParsePacketAdditionalData(int32 packetAdditionalSize, double& outTimestamp)
	{
		outTimestamp = 0.0;

		if ((_fileHeader.snifferId != 0x15 && _fileHeader.snifferId != 0x16) || packetAdditionalSize <= 0)
			return;

		size_t startPos = _file.tellg();

		_file.read(reinterpret_cast<char*>(&outTimestamp), sizeof(outTimestamp));

		if (_fileHeader.snifferVersion >= 0x0101)
		{
			uint8 commentLength;
			_file.read(reinterpret_cast<char*>(&commentLength), sizeof(commentLength));

			if (commentLength > 0)
			{
				// or just skip?
				std::vector<char> comment(commentLength);
				_file.read(comment.data(), commentLength);
				LOG("Read {} byte message", commentLength);
			}
		}

		if (_fileHeader.snifferVersion >= 0x0102)
		{
			size_t currentPos = _file.tellg();
			size_t bytesRead = currentPos - startPos;

			while (bytesRead < static_cast<size_t>(packetAdditionalSize))
			{
				uint8 type;
				_file.read(reinterpret_cast<char*>(&type), sizeof(type));
				bytesRead++;

				switch (type)
				{
				case 0x50: // override phasing
				{
					uint64 lowGuid, highGuid;
					int32 phaseId;
					_file.read(reinterpret_cast<char*>(&lowGuid), sizeof(lowGuid));
					_file.read(reinterpret_cast<char*>(&highGuid), sizeof(highGuid));
					_file.read(reinterpret_cast<char*>(&phaseId), sizeof(phaseId));
					bytesRead += 8 + 8 + 4;
					LOG("Phase override: GUID({:016X}{:016X}), Phase: {}", highGuid, lowGuid, phaseId);
					break;
				}
				default:
				{
					// skip unk type
					size_t remaining = packetAdditionalSize - bytesRead;
					if (remaining > 0)
					{
						_file.seekg(remaining, std::ios::cur);
						LOG("UNK type 0x{:02X}, skipped {} bytes", type, remaining);
					}
					bytesRead = packetAdditionalSize;
					break;
				}
				}

				currentPos = _file.tellg();
				bytesRead = currentPos - startPos;
			}
		}
	}

}

