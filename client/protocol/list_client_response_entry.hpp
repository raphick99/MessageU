#pragma once

#include <array>
#include <cstdint>

namespace Protocol
{
#pragma pack(push, 1)
	struct ListClientResponseEntry
	{
		std::array<uint8_t, 16> client_id;
		std::array<char, 255> name;
	};
#pragma pack(pop)
}

