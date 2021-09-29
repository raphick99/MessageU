#pragma once

#include <cstdint>

namespace Protocol
{
	enum class ResponseCode : uint16_t
	{
		Register = 2000,
		ListUsers = 2001,
		GetPublicKey = 2002,
		MessageSend = 2003,
		PullMessages = 2004,
		GeneralError = 9000,
	};
}

