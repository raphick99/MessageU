#pragma once

#include "client_id.hpp"

namespace Protocol
{
#pragma pack(push, 1)
	struct GetPublicKeyRequest
	{
		ClientID client_id;
	};
#pragma pack(pop)
}
