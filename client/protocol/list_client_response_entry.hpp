#pragma once

#include <array>
#include "client_id.hpp"

namespace Protocol
{
#pragma pack(push, 1)
	struct ListClientResponseEntry
	{
		ClientID client_id;
		std::array<char, 255> name;
	};
#pragma pack(pop)
}

