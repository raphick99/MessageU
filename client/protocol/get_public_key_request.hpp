#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct GetPublicKeyRequest
	{
		std::array<uint8_t, 16> client_id;
	};
#pragma pack(pop)
}
