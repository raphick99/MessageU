#pragma once

#include "client_id.h"

namespace Protocol
{
#pragma pack(push, 1)
	struct RegisterResponse
	{
		ClientID client_id;
	};
#pragma pack(pop)
}

