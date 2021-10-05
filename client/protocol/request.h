#pragma once

#include <cstdint>
#include "client_id.h"
#include "request_code.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct RequestHeader
	{
		ClientID client_id;
		uint8_t version;
		RequestCode request_code;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

