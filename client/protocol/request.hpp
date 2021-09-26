#pragma once

#include <array>
#include <cstdint>
#include "request_code.hpp"

namespace Protocol
{
#pragma pack(push, 1)
	struct RequestHeader
	{
		std::array<uint8_t, 16> client_id;
		uint8_t version;
		RequestCode request_code;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

