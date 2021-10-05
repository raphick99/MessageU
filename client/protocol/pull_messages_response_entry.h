#pragma once

#include <cstdint>
#include "client_id.h"
#include "message_type.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct PullMessagesResponseEntry
	{
		ClientID client_id;
		uint32_t message_id;
		MessageType message_type;
		uint32_t payload_size;
	};
#pragma pack(pop)
}

