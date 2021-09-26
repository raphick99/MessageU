#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct RegisterResponse
	{
		std::array<uint8_t, 16> uuid;
	};
#pragma pack(pop)
}

