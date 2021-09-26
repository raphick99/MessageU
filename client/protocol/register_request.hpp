#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct RegisterRequest
	{
		std::array<char, 255> name;
		std::array<uint8_t, 160> public_key;
	};
#pragma pack(pop)
}

