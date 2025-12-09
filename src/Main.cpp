#include <fmt/core.h>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <thread>

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
				const int ITERS = 100000;
				
				std::vector<std::vector<uint8>> spellPackets;
				spellPackets.push_back(pkt.data);
				while (pktOpt.has_value())
				{
					pktOpt = reader.ReadNextPacket();
					if (pktOpt.has_value())
					{
						Pkt const& nextPkt = pktOpt.value();
						if (nextPkt.header.opcode == static_cast<uint32>(Opcode::SMSG_SPELL_GO))
						{
							spellPackets.push_back(nextPkt.data);
							if (spellPackets.size() >= 2000)
								break;
						}
					}
				}
				LOG("Found {} SMSG_SPELL_GO packets", spellPackets.size());

				// cpu warmup
				for (int i = 0; i < 100; i++)
				{
					for (auto const& data : spellPackets)
					{
						BitReader r(data.data(), data.size());
						Parser::ParseSpellGo_CHUNKED(r);
					}
				}

				auto start1 = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < ITERS; i++)
				{
					for (auto const& data : spellPackets)
					{
						BitReader r(data.data(), data.size());
						Parser::ParseSpellGo_CHUNKED(r);
					}
				}
				auto end1 = std::chrono::high_resolution_clock::now();
				auto total1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1);

				const int NUM_THREADS = std::thread::hardware_concurrency();
				LOG("Running threaded benchmark with {} threads...", NUM_THREADS);

				auto start2 = std::chrono::high_resolution_clock::now();

				std::vector<std::thread> threads;
				for (int t = 0; t < NUM_THREADS; ++t)
				{
					threads.emplace_back([&, t]()
					{
						for (int iter = 0; iter < ITERS; iter++)
						{
							for (size_t i = t; i < spellPackets.size(); i += NUM_THREADS)
							{
								BitReader r(spellPackets[i].data(), spellPackets[i].size());
								Parser::ParseSpellGo_CHUNKED(r);
							}
						}
					});
				}

				for (auto& thread : threads)
					thread.join();

				auto end2 = std::chrono::high_resolution_clock::now();
				auto total2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2);

				size_t totalPackets = ITERS * spellPackets.size();
				LOG("Single-threaded: {}ns average ({} packets)", total1.count() / totalPackets, totalPackets);
				LOG("Multi-threaded:  {}ns average ({} packets, {} threads)", total2.count() / totalPackets, totalPackets, NUM_THREADS);
				LOG("Speedup:         {:.2f}x", (double)total1.count() / total2.count());

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