#include "WowGuid.h"

#include <fmt/core.h>

namespace PktParser::Misc
{
	GuidType WowGuid128::GetType() const
	{
		return static_cast<GuidType>((High >> 58) & 0x3F);
	}

	uint16 WowGuid128::GetRealmId() const
	{
		return (High >> 42) & 0x1FFF;
	}

	uint16 WowGuid128::GetMapId() const
	{
		return (High >> 29) & 0x1FFF;
	}

	uint32 WowGuid128::GetEntry() const
	{
		return (High >> 6) & 0x7FFFFF;
	}

	uint8 WowGuid128::GetSubType() const
	{
		return High & 0x3F;
	}

	uint32 WowGuid128::GetServerId() const
	{
		return (Low >> 40) & 0xFFFFFF;
	}

	uint64 WowGuid128::GetLow() const
	{
		return Low & 0xFFFFFFFFFF;
	}

	bool WowGuid128::IsEmpty() const
	{
		return Low == 0 && High == 0;
	}

	bool WowGuid128::HasEntry() const
	{
		return GuidTypeHasEntry(GetType());
	}

	std::string WowGuid128::ToString() const
	{
		if (IsEmpty())
			return "Full: 0x0";

		if (HasEntry())
		{
			return fmt::format("Full: 0x{:016X}{:016X} {}/{} R{}/S{} Map: {} Entry: {} Low: {}", High, Low,
				GuidTypeToString(GetType()), GetSubType(), GetRealmId(), GetServerId(), GetMapId(), GetEntry(), GetLow());
		}

		return fmt::format("Full: 0x{:016X}{:016X} {}/{} R{}/S{} Map: {} Low: {}", High, Low,
			GuidTypeToString(GetType()), GetSubType(), GetRealmId(), GetServerId(), GetMapId(), GetLow());
	}

    std::string WowGuid128::ToHexString() const
    {
		if (IsEmpty())
			return "0";
		return fmt::format("{:016X}{:016X}", High, Low);
    }

    WowGuid128 ReadGuid128(BitReader& reader)
	{
		WowGuid128 guid;
		guid.Low = reader.ReadUInt64();
		guid.High = reader.ReadUInt64();
		return guid;
	}

	WowGuid128 ReadPackedGuid128(BitReader& reader)
	{
		uint8 lowMask = reader.ReadUInt8();
		uint8 highMask = reader.ReadUInt8();

		WowGuid128 guid{ 0, 0 };

		for (int i = 0; i < 8; ++i)
			if (lowMask & 1 << i)
			{
				uint8 byte = reader.ReadUInt8();
				guid.Low |= (static_cast<uint64>(byte) << (i * 8));
			}

		for (int i = 0; i < 8; ++i)
			if (highMask & 1 << i)
			{
				uint8 byte = reader.ReadUInt8();
				guid.High |= (static_cast<uint64>(byte) << (i * 8));
			}

		return guid;
	}
}