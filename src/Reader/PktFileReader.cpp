#include "pchdef.h"
#include "PktFileReader.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace PktParser::Enums;

namespace PktParser::Reader
{
	PktFileReader::PktFileReader(std::string const& filepath)
		: _fd{ -1 }, _mappedData{ nullptr }, _fileSize{ 0 }, _position{ 0 },
		_filepath{ filepath }, _fileHeader{}, _pktNumber{ 0 }
	{
		
		_fd = open(filepath.c_str(), O_RDONLY);
		if (_fd < 0)
			throw ParseException{ "Failed to open: " + filepath };

		struct stat sb;
		if (fstat(_fd, &sb) < 0)
		{
			close(_fd);
			throw ParseException{ "Failed to stat: " + filepath };
		}
		
		_fileSize = static_cast<size_t>(sb.st_size);

		if (_fileSize == 0)
		{
			close(_fd);
			throw ParseException{ "Empty file: " + filepath };
		}

		void* mapped = mmap(nullptr, _fileSize, PROT_READ, MAP_PRIVATE, _fd, 0);
		if (mapped == MAP_FAILED)
		{
			close(_fd);
			throw ParseException{ "Failed to mmap: " + filepath };
		}

		_mappedData = static_cast<uint8 const*>(mapped);

		madvise(const_cast<uint8*>(_mappedData), _fileSize, MADV_SEQUENTIAL);
	}

	PktFileReader::~PktFileReader()
	{
		if (_mappedData)
			munmap(const_cast<uint8*>(_mappedData), _fileSize);

		if (_fd >= 0)
			close(_fd);
	}

	void PktFileReader::ParseFileHeader()
	{
		char magik[3];
		ReadInto(magik, 3);

		if (!std::equal(magik, magik + 3, "PKT"))
			throw ParseException{ "Invalid PKT file" };

		_fileHeader.version = Read<uint16>();
		_fileHeader.snifferId = Read<uint8>();
		_fileHeader.clientBuild = Read<uint32>();
		ReadInto(_fileHeader.locale, 4);
		Skip(40); // skip session key
		_fileHeader.startTime = Read<uint32>();
		_fileHeader.startTickCount = Read<uint32>();
		int32 additionalLength = Read<int32>();

		_fileHeader.snifferVersion = 0;
		if ((_fileHeader.snifferId == 0x15 || _fileHeader.snifferId == 0x16) && additionalLength >= 2)
			_fileHeader.snifferVersion = Read<uint16>();
		else
			Skip(additionalLength);
	}

	std::optional<Pkt> PktFileReader::ReadNextPacket()
	{
		if (AtEnd())
        	return std::nullopt;

		try
		{
			PktHeader header = ParsePacketHeader();

			if (_position + header.packetLength > _fileSize)
            	return std::nullopt;

			std::vector<uint8> pktData(header.packetLength);
			ReadInto(pktData.data(), header.packetLength);

			if (header.packetLength >= 4)
				header.opcode = static_cast<uint32>(pktData[0])
					| (static_cast<uint32>(pktData[1]) << 8)
					| (static_cast<uint32>(pktData[2]) << 16)
					| (static_cast<uint32>(pktData[3]) << 24);
			else
				header.opcode = 0;
			
			Pkt pkt{ header, std::move(pktData) };
			pkt.pktNumber = _pktNumber;
			_pktNumber++;
			return pkt;
		}
		catch (ParseException const&)
		{
			return std::nullopt;
		}
		catch (std::exception const& e)
		{
			LOG("Unexpected error reading packet {}: {}", _pktNumber, e.what());
			return std::nullopt;
		}
	}

	PktHeader PktFileReader::ParsePacketHeader()
	{
		PktHeader header{};

		uint32 directionMagik = Read<uint32>();

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

		header.connectionIndex = Read<int32>();
		header.tickCount = Read<uint32>();

		int32 packetAdditionalSize = Read<int32>();
		header.packetLength = Read<int32>();

		ParsePacketAdditionalData(packetAdditionalSize, header.timestamp);

		return header;
	}

	void PktFileReader::ParsePacketAdditionalData(int32 packetAdditionalSize, double& outTimestamp)
	{
		outTimestamp = 0.0;

		if ((_fileHeader.snifferId != 0x15 && _fileHeader.snifferId != 0x16) || packetAdditionalSize <= 0)
			return;

		size_t bytesRead = 0;

		outTimestamp = Read<double>();
		bytesRead += sizeof(double);

		if (_fileHeader.snifferVersion >= 0x0101)
		{
			uint8 commentLength = Read<uint8>();
			bytesRead += sizeof(uint8);

			if (commentLength > 0)
			{
				// or just skip?
				Skip(commentLength);
				bytesRead += commentLength;
			}
		}

		if (_fileHeader.snifferVersion >= 0x0102)
		{
			while (bytesRead < static_cast<size_t>(packetAdditionalSize))
			{
				uint8 type = Read<uint8>();
				bytesRead++;

				switch (type)
				{
				case 0x50: // override phasing
				{
					uint64 lowGuid = Read<uint64>();
					uint64 highGuid = Read<uint64>();
					int32 phaseId = Read<int32>();
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
						Skip(remaining);
						bytesRead += remaining;
						LOG("UNK type 0x{:02X}, skipped {} bytes", type, remaining);
					}
					break;
				}
				}
			}
		}

		size_t remaining = packetAdditionalSize - bytesRead;
		if (remaining > 0)
			Skip(remaining);
	}
}

