#pragma once

#include <array>
#include <cstdint>
#include "client_id.hpp"
#include "message_type.hpp"

namespace Protocol
{
#pragma pack(push, 1)
	struct SendMessageRequest
	{
		ClientID client_id;
		MessageType messsage_type;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

