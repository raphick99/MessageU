#pragma once

#include <cstdint>
#include "response_code.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct ResponseHeader
	{
		uint8_t version;
		ResponseCode response_code;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

