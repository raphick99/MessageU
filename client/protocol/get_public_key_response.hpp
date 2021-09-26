#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct GetPublicKeyResponse
	{
		std::array<uint8_t, 16> client_id;
		std::array<uint8_t, 160> public_key;
	};
#pragma pack(pop)
}
