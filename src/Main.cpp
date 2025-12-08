#include <fstream>
#include <cstdint>
#include <filesystem>
#include <fmt/core.h>

#include "Misc/Utilities.h"
#include "Misc/Logger.h"
#include "Reader/BitReader.h"
#include "Reader/PktFileReader.h"
#include "Parser/Parser.h"

using namespace PktParser;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		fmt::print(stderr, "Usage: {} <pkt_file>\n", argv[0]);
		fmt::print(stderr, "Or drag and drop a .pkt file onto the executable\n");
		return 1;
	}

	try
	{
		PktFileReader reader(argv[1]);
		reader.ParseFileHeader();
		
		LOG("----- Reading Packets -----");
		LOG("");

		std::optional<Pkt> pktOpt = reader.ReadNextPacket();
		while (pktOpt.has_value())
		{
			Pkt const& pkt = pktOpt.value();

			std::string opcodeName = GetOpcodeName(pkt.header.opcode);

			LOG("{}: {} Opcode 0x{:06X} | Length: {} | ConnIdx: {} | Time: {} | Number: {}",
				pkt.header.direction,
				opcodeName,
				pkt.header.opcode,
				pkt.header.packetLength - 4,
				pkt.header.connectionIndex,
				Utilities::FormatUnixMilliseconds(pkt.header.timestamp),
				reader.GetPacketNumber() - 1
			);

			if (pkt.header.opcode == static_cast<uint32>(Opcode::SMSG_AUTH_CHALLENGE))
			{
				BitReader packetReader = pkt.CreateReader();
				Parser::ParseAuthChallenge(packetReader);
			}
			else if (pkt.header.opcode == static_cast<uint32>(Opcode::SMSG_SPELL_GO))
			{
				BitReader packetReader = pkt.CreateReader();
				Parser::ParseSpellGo(packetReader);
				break;
			}

			pktOpt = reader.ReadNextPacket();
		}

		LOG(">>>>> PARSE COMPLETE <<<<<");
	}
	catch (std::exception const& e)
	{
		LOG("Error: {}", e.what());
		return 1;
	}

#ifdef _WIN32
	std::cin.get();
#endif
}