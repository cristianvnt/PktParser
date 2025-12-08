#ifndef WOW_GUID_H
#define WOW_GUID_H

#include "Misc/Define.h"
#include "Reader/BitReader.h"

namespace PktParser
{
	struct WowGuid128
	{
		uint64 Low;
		uint64 High;

		uint8 GetType() const;
		uint16 GetRealmId() const;
		uint16 GetMapId() const;
		uint32 GetEntry() const;
		uint8 GetSubType() const;
		uint32 GetServerId() const;
		uint64 GetLow() const;
		bool IsEmpty() const;
	};

	WowGuid128 ReadGuid128(BitReader& reader);
	WowGuid128 ReadPackedGuid128(BitReader& reader);
}

#endif // WOW_GUID_H