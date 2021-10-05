#pragma once

#include <cstdint>

namespace Protocol
{
	enum class RequestCode : uint16_t
	{
		Register = 1000,
		ListUsers = 1001,
		GetPublicKey = 1002,
		MessageSend = 1003,
		PullMessages = 1004,
	};
}

