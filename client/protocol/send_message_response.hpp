#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct SendMessageResponse
	{
		std::array<uint8_t, 16> client_id;
		uint32_t message_id;
	};
#pragma pack(pop)
}
