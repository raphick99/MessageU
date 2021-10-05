#pragma once

#include <array>
#include <cstdint>
#include "client_id.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct GetPublicKeyResponse
	{
		ClientID client_id;
		std::array<uint8_t, 160> public_key;
	};
#pragma pack(pop)
}
