#pragma once

#include <array>
#include <cstdint>
#include "message_type.hpp"

namespace Protocol
{
#pragma pack(push, 1)
	struct PullMessagesResponseEntry
	{
		std::array<uint8_t, 16> client_id;
		uint32_t message_id;
		MessageType message_type;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

