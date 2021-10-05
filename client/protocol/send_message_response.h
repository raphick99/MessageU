#pragma once

#include <cstdint>
#include "client_id.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct SendMessageResponse
	{
		ClientID client_id;
		uint32_t message_id;
	};
#pragma pack(pop)
}
